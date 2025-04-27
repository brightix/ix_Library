import cv2
import numpy as np

# cv2.namedWindow('window',cv2.WINDOW_NORMAL)
#
# cv2.resizeWindow('window',640,480)

img = cv2.imread('./coins.png')
img = cv2.resize(img,None,fx=0.4,fy=0.4)

gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
_,gray = cv2.threshold(gray,0,255,cv2.THRESH_BINARY_INV | cv2.THRESH_OTSU)

kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE,(3,3))

opening = cv2.morphologyEx(gray,cv2.MORPH_OPEN,kernel)

bg = cv2.dilate(opening,kernel,iterations=2)

fg = cv2.erode(bg,kernel,iterations=2)

distanceTransform =  cv2.distanceTransform(opening,cv2.DIST_L2,5)

cv2.normalize(distanceTransform,distanceTransform,0,1.0,cv2.NORM_MINMAX)

_,fg = cv2.threshold(distanceTransform,0.5*distanceTransform.max(),255,cv2.THRESH_BINARY)

fg = np.uint8(fg)
unknown = cv2.subtract(bg,fg)

_,markers = cv2.connectedComponents(fg)

markers += 1

print(markers.max(),markers.min())
markers_copy = markers.copy()
markers[unknown == 255] = 0

print(markers.min())
markers_copy[markers == 0] = 150
markers_copy[markers == 1] = 0
markers_copy[markers > 1] = 255

markers_copy = np.uint8(markers_copy)

markers = cv2.watershed(img,markers)

#img[markers == -1] = [0,0,255]

#img[markers > 1] = [0,255,0]

mask = np.zeros(shape=img.shape[:2],dtype=np.uint8)
mask[markers > 1] = 255
coin = cv2.bitwise_and(img,img,mask=mask)


img2 = cv2.imread('./coins.png')
coin_canny = cv2.Canny(coin,100,255)



img2_gray = cv2.cvtColor(img2,cv2.COLOR_BGR2GRAY)
_,img2_gray = cv2.threshold(img2_gray,0,255,cv2.THRESH_BINARY_INV | cv2.THRESH_OTSU)

contours,_ = cv2.findContours(img2_gray,cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)

cv2.drawContours(img2,contours,-1,(0,0,255),3)

#cv2.imshow('coin',np.hstack((gray,bg,distanceTransform,fg,unknown)))
cv2.imshow('img',img2)
cv2.waitKey(0)
cv2.destroyAllWindows()