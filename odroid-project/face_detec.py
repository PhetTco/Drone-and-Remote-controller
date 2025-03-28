import cv2
import face_recognition
import time
import os
import requests  # Required to send POST requests for Line Notify
from simple_facerec import SimpleFacerec

# Function to send notification via Line Notify
def send_line_notify(token, message, image_path=None):
    url = "https://notify-api.line.me/api/notify"
    headers = {"Authorization": f"Bearer {token}"}
    data = {"message": message}
    
    if image_path:
        files = {"imageFile": open(image_path, "rb")}
        response = requests.post(url, headers=headers, data=data, files=files)
    else:
        response = requests.post(url, headers=headers, data=data)
    
    return response.status_code

# Load face encodings from the Pictures directory on Odroid
sfr = SimpleFacerec()
sfr.load_encoding_images("/home/odroid/Pictures/")

# Path to save unknown faces
unknown_save_path = "/home/odroid/Pictures/unknownPic/"

# Create the directory if it doesn't exist
if not os.path.exists(unknown_save_path):
    os.makedirs(unknown_save_path)

# Initialize camera
cap = cv2.VideoCapture(0)

# Counter for naming unknown images
unknown_counter = 0

# Variable to track time since last unknown person screenshot
last_screenshot_time = 0
screenshot_interval = 5  # Time in seconds between screenshots

# Line Notify token (replace with your actual token)
line_notify_token = "CEZVHWG73lixUrhTDBRf4CK1qE1o7Ll3FqBTcBeUKv9"

while True:
    ret, frame = cap.read()

    if not ret:
        print("Failed to grab frame")
        break

    # Detect Faces
    face_locations, face_names = sfr.detect_known_faces(frame)
    
    for face_loc, name in zip(face_locations, face_names):
        y1, x2, y2, x1 = face_loc[0], face_loc[1], face_loc[2], face_loc[3]
        
        # Set name text color to yellow (BGR: 0, 255, 255)
        cv2.putText(frame, name, (x1, y1 - 10), cv2.FONT_HERSHEY_DUPLEX, 1, (0, 255, 255), 2)
        
        # Set rectangle color to white (BGR: 255, 255, 255)
        cv2.rectangle(frame, (x1, y1), (x2, y2), (255, 255, 255), 4)

        # If the name is 'Unknown', save the screenshot and send notification
        if name == "Unknown":
            current_time = time.time()
            # Check if it's been at least 5 seconds since the last screenshot
            if current_time - last_screenshot_time > screenshot_interval:
                # Save the image of the unknown person
                unknown_img_path = os.path.join(unknown_save_path, f"unknown_{unknown_counter}.jpg")
                cv2.imwrite(unknown_img_path, frame)
                print(f"Unknown person saved as {unknown_img_path}")
                
                # Send a Line Notify message with the image
                response_code = send_line_notify(line_notify_token, "Unknown person detected!", unknown_img_path)
                if response_code == 200:
                    print("Line notification sent successfully.")
                else:
                    print("Failed to send Line notification.")
                
                unknown_counter += 1
                last_screenshot_time = current_time  # Update the time of the last screenshot

    # Display the frame
    cv2.imshow("Frame", frame)

    # Press 'q' to exit the loop
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release the camera and close all windows
cap.release()
cv2.destroyAllWindows()


