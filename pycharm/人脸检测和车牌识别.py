import cv2
import numpy as np

cap = cv2.VideoCapture('./video_1.mp4')
img = cv2.imread('./girl1.png')
cv2.namedWindow('window',cv2.WINDOW_NORMAL)
cv2.resizeWindow('window',640,480)

#facer = cv2.CascadeClassifier('./haarcascades/haarcascade_frontalface_alt2.xml')
eye = cv2.CascadeClassifier('./haarcascades/haarcascade_eye.xml')
body = cv2.CascadeClassifier('./haarcascades/haarcascade_upperbody.xml')


# img_gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
# faces = body.detectMultiScale(img_gray,scaleFactor=1.2,minNeighbors=4)
# for(x,y,w,h) in faces:
#     cv2.rectangle(img,(x,y),(x+w,y+h),(0,0,255),3)
# cv2.imshow('window',img)

while True:
    ret,frame = cap.read()
    if not ret:
        break
    frame_gray = cv2.cvtColor(frame,cv2.COLOR_BGR2GRAY)
    faces = body.detectMultiScale(frame_gray,scaleFactor=1.2,minNeighbors=4)
    for(x,y,w,h) in faces:
        cv2.rectangle(frame,(x,y),(x+w,y+h),(0,0,255),3)
    cv2.imshow('window',frame)
    if cv2.waitKey(5) == ord('q'):
        break
cv2.waitKey(0)
cv2.destroyAllWindows()
