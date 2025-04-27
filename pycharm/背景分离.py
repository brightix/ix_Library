import cv2
import numpy as np

cap = cv2.VideoCapture('./video_1.mp4')
cv2.namedWindow('window',cv2.WINDOW_NORMAL)
cv2.resizeWindow('window',640,480)
gmg = cv2.createBackgroundSubtractorMOG2()

while True:
    ret,frame = cap.read()
    if not ret:
        break

    fgmask = gmg.apply(frame)
    cv2.imshow('window',fgmask)
    if cv2.waitKey(25) == ord('q'):
        break
cv2.waitKey(0)
cv2.destroyAllWindows()