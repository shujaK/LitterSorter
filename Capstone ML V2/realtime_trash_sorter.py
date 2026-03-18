import time
from pathlib import Path
from typing import Tuple, Dict, Optional

import cv2
import numpy as np
import torch
from torchvision import transforms
from torch import nn
import serial

from send_signal import pack_start_signal


MODEL_PATH = Path("models/trash_classifier.pth")

# Adjust this to the correct USB CDC device for STM32 board.
#something like "/dev/tty.usbmodemXXXX".
SERIAL_PORT = "COM6"
BAUD_RATE = 115200

# Map predicted labels to single-character commands.
LABEL_TO_CHAR: Dict[str, str] = {
    "metal": "M",
    "paper": "A",   #'A' for 'paper'
    "plastic": "P",
}

# Map predicted labels to enum values for binary protocol
LABEL_TO_ENUM: Dict[str, int] = {
    "metal": 0,
    "paper": 2,
    "plastic": 1,
}


def load_model(device: str = "cuda" if torch.cuda.is_available() else "cpu") -> Tuple[nn.Module, list, str]:
    checkpoint = torch.load(MODEL_PATH, map_location=device)
    class_names = checkpoint.get("class_names", ["metal", "paper", "plastic"])

    from torchvision import models

    model = models.resnet18(weights=None)
    in_features = model.fc.in_features
    model.fc = nn.Linear(in_features, len(class_names))
    model.load_state_dict(checkpoint["model_state_dict"])
    model.to(device)
    model.eval()

    return model, class_names, device


def open_serial() -> Optional[serial.Serial]:
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Opened serial port {SERIAL_PORT} at {BAUD_RATE} baud")
        return ser
    except Exception as e:
        print(f"Warning: could not open serial port {SERIAL_PORT}: {e}")
        return None


def send_serial_command(ser: Optional[serial.Serial], label: str):
    ch = LABEL_TO_CHAR.get(label)
    litter_enum = LABEL_TO_ENUM.get(label)
    
    if ch is None or litter_enum is None:
        return
    
    # Pack binary signal: speed=50, duration=1000 (can be adjusted)
    binary_packet = pack_start_signal(speed=50, duration=1000, litter=litter_enum)
    
    if ser is not None and ser.is_open:
        try:
            bytes_sent = ser.write(binary_packet)
            ser.flush()
            print(f"Sent signal for {label} over serial: {binary_packet.hex()} ({bytes_sent} bytes)")
        except Exception as e:
            print(f"Error sending over serial: {e}")
    else:
        print(f"(Serial closed) Would send signal for {label}: {binary_packet.hex()}")


def main():
    model, class_names, device = load_model()

    transform = transforms.Compose(
        [
            transforms.Resize((224, 224)),
            transforms.ToTensor(),
            transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]),
        ]
    )

    cap = cv2.VideoCapture(1)
    if not cap.isOpened():
        print("Error: could not open webcam.")
        return

    bg_subtractor = cv2.createBackgroundSubtractorMOG2(history=300, varThreshold=50, detectShadows=True)

    ser = open_serial()

    last_label: Optional[str] = None
    last_conf: float = 0.0
    last_sent_time: float = 0.0
    sent_for_current_object: bool = False

    classification_done_until: float = 0.0  # timestamp until which we show the "completed" message
    COMPLETED_DISPLAY_SECS = 2.0

    MIN_OBJECT_AREA = 5000  # pixels; tune based on your setup

    # How many consecutive frames the object must be present before classification.
    STABLE_FRAMES = 8
    stable_counter = 0

    # How many consecutive "no object" frames before we consider the object gone.
    ABSENT_RESET_FRAMES = 10
    absent_counter = 0

    # Smoothing over predictions for the current object.
    PRED_HISTORY_LEN = 8
    MIN_MAJORITY_FRACTION = 0.6
    MIN_CONFIDENCE = 0.70
    pred_history = []

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                break

            frame_gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            fg_mask = bg_subtractor.apply(frame_gray)

            # Remove noise and small artifacts.
            kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (5, 5))
            fg_mask = cv2.morphologyEx(fg_mask, cv2.MORPH_OPEN, kernel)
            fg_mask = cv2.morphologyEx(fg_mask, cv2.MORPH_DILATE, kernel, iterations=2)

            contours, _ = cv2.findContours(fg_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

            object_present = False
            roi = None
            roi_box = None

            if contours:
                largest = max(contours, key=cv2.contourArea)
                area = cv2.contourArea(largest)
                if area > MIN_OBJECT_AREA:
                    x, y, w, h = cv2.boundingRect(largest)
                    # Add a margin around the detected object.
                    margin = 20
                    x = max(x - margin, 0)
                    y = max(y - margin, 0)
                    x2 = min(x + w + 2 * margin, frame.shape[1])
                    y2 = min(y + h + 2 * margin, frame.shape[0])
                    roi = frame[y:y2, x:x2]
                    roi_box = (x, y, x2, y2)
                    object_present = True

            if object_present and roi is not None:
                stable_counter += 1
                absent_counter = 0
            else:
                absent_counter += 1

                # Only fully reset once the object has been gone for a few frames,
                # to avoid flicker causing multiple "objects".
                if absent_counter >= ABSENT_RESET_FRAMES:
                    stable_counter = 0
                    sent_for_current_object = False
                    last_label = None
                    last_conf = 0.0
                    pred_history = []

            # Default state is waiting; we may override it below.
            state_text = "WAITING"

            if stable_counter >= STABLE_FRAMES and roi is not None:
                # Classify the object.
                roi_rgb = cv2.cvtColor(roi, cv2.COLOR_BGR2RGB)
                roi_pil = cv2.resize(roi_rgb, (224, 224))
                roi_pil = transforms.ToPILImage()(roi_pil)

                input_tensor = transform(roi_pil).unsqueeze(0).to(device)
                with torch.no_grad():
                    outputs = model(input_tensor)
                    probs = torch.softmax(outputs, dim=1)[0]
                    conf, pred_idx = torch.max(probs, 0)

                label = class_names[pred_idx.item()]
                conf_val = float(conf.item())

                # Update prediction history for current object.
                pred_history.append((label, conf_val))
                if len(pred_history) > PRED_HISTORY_LEN:
                    pred_history.pop(0)

                # Compute majority label and average confidence.
                counts: Dict[str, int] = {}
                conf_sums: Dict[str, float] = {}
                for lbl, c in pred_history:
                    counts[lbl] = counts.get(lbl, 0) + 1
                    conf_sums[lbl] = conf_sums.get(lbl, 0.0) + c

                majority_label = max(counts, key=counts.get)
                majority_count = counts[majority_label]
                majority_fraction = majority_count / len(pred_history)
                avg_conf = conf_sums[majority_label] / majority_count

                # Only accept and send when predictions are stable and confident.
                if (
                    not sent_for_current_object
                    and majority_fraction >= MIN_MAJORITY_FRACTION
                    and avg_conf >= MIN_CONFIDENCE
                ):
                    last_label = majority_label
                    last_conf = avg_conf

                    send_serial_command(ser, majority_label)
                    sent_for_current_object = True
                    last_sent_time = time.time()
                    classification_done_until = last_sent_time + COMPLETED_DISPLAY_SECS

                # While we are within the "completed" window, show a completion state.
                now = time.time()
                if now <= classification_done_until and sent_for_current_object:
                    state_text = "COMPLETED"
                else:
                    state_text = "CLASSIFYING"

                # Draw ROI box on original frame.
                if roi_box is not None:
                    x1, y1, x2, y2 = roi_box
                    cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)

            # Overlay text.
            if last_label is not None:
                text = f"Classification completed! This material is {last_label} ({last_conf * 100:.1f}%)"
            else:
                text = "No item detected"

            cv2.putText(
                frame,
                f"State: {state_text}",
                (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.8,
                (255, 255, 255),
                2,
            )
            cv2.putText(
                frame,
                text,
                (10, 60),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.8,
                (0, 255, 255),
                2,
            )

            cv2.imshow("Trash Sorter", frame)

            key = cv2.waitKey(1) & 0xFF
            if key == ord("q"):
                break

    finally:
        cap.release()
        cv2.destroyAllWindows()
        if ser is not None and ser.is_open:
            ser.close()


if __name__ == "__main__":
    main()

