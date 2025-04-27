import cv2
import numpy as np

img1 = cv2.imread('./girl1_body.png')
template = cv2.imread('./girl1_head.png')

print(img1.shape)
print(template.shape)
res = cv2.matchTemplate(img1,template,cv2.TM_SQDIFF)
print(res.shape)
min_val,max_val,min_loc,max_loc = cv2.minMaxLoc(res)

cv2.rectangle(img1, min_loc,(min_loc[0] + template.shape[1] ,min_loc[1] + template.shape[0]), (0,0,255), 2)
cv2.imshow('img1',img1)
cv2.imshow('template',template)
cv2.waitKey(0)
cv2.destroyAllWindows()


