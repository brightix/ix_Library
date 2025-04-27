import cv2
import numpy as np

cap = cv2.VideoCapture('./video_1.mp4')
cv2.namedWindow('window',cv2.WINDOW_NORMAL)
cv2.resizeWindow('window',640,480)
while True:
    ret,frame = cap.read()
    if not ret:
        break

    cv2.imshow('frame',frame)
    key = cv2.waitKey(25)
    if key == ord('q'):
       break
    elif key == ord('s'):
        roi = cv2.selectROI('frame',frame,showCrosshair=False,fromCenter=False)
cap.release()
cv2.waitKey(0)
cv2.destroyAllWindows()