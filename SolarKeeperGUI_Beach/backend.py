from tkinter import *
from time import sleep
import locale
import functools
from datetime import datetime
import cv2
from PIL import Image, ImageTk

import requests
from bs4 import BeautifulSoup


def setUVColor(UVI):
    if UVI<=2:
        return 'green'
    elif (3 <= UVI <= 7):
        return 'orange'
    else:
        return 'red'

def setTextInfo(UVI):
    if UVI<=2:
        return "Baixo\nLow"
    elif (3 <= UVI<= 5):
        return 'Moderado\nModerate'
    elif (6 <= UVI <= 7):
        return 'Elevado\nHigh'
    elif (8 <= UVI <= 10):
        return 'Muito Elevado\nVery High'
    else:
        return 'Extremo\nExtreme'

def convertCtoF(tempC):
    if tempC < -273.15:
        return "That temperature doesn't make sense!"
    else:
        tempF = tempC* 9/5 + 32
        return tempF

def getValues():
    values = []
    try:
        with requests.Session() as s:
            r = s.get("http://192.168.4.1:8080", timeout = 5 )
            c=r.content
            soup = BeautifulSoup(c,"html.parser")
            print("soup :", soup)
            dataList = str(soup).split(' ')
            print(dataList)
    except Exception as exc:
       print("ERROR: Timeout")
       print(exc)
       #dataList = "6000 13 22.30 60 17.00 7.00".split(" ")
    for i in dataList:
        values.append(float(i))
    print(values)
    if values[1] > 11:
        values[1] = 11
    else if values[1] < 0:
        values[1] = 0
    return values
