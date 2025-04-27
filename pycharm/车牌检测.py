import cv2
import numpy as np
import pytesseract

img = cv2.imread('./车牌.png')
cv2.namedWindow('window',cv2.WINDOW_NORMAL)
cv2.resizeWindow('window',640,480)
plate_number = cv2.CascadeClassifier('./haarcascades/haarcascade_russian_plate_number.xml')

img_gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
faces = plate_number.detectMultiScale(img_gray,scaleFactor=1.1,minNeighbors=4)
for(x,y,w,h) in faces:
    if h > 90:
        cv2.rectangle(img,(x,y),(x+w,y+h),(0,0,255),2)
        roi = img_gray[y: y+h,x: x+w]
        ret,result = cv2.threshold(roi,0,255,cv2.THRESH_BINARY | cv2.THRESH_OTSU)


        kernel = np.ones(shape=(3,3),dtype=np.uint8)

        roi = cv2.morphologyEx(result,cv2.MORPH_OPEN,kernel)
        print(pytesseract.image_to_string(roi,lang='chi_sim+eng',config='--psm 8 --oem 3'))
        cv2.imshow('window',roi)


#cv2.imshow('window',img)



cv2.waitKey(0)
cv2.destroyAllWindows()
