#!/usr/bin/env python3
# coding=utf-8

from backend import *

#values = [0:uvValueA 1:uVindex 2:tempAr 3:HumAr 4:tempAgua 5:pH 6:ConcentrCloro]
values = [0, 0, 18, 0, 14.3, 7.3, 9.8]


window = Tk()

windowValue = Frame(window)
framePUB = Frame(window)
labelPUB = None

frameWarning = Frame(window)
labelWarning = None

varUV = StringVar()
varUV.set(str().zfill(2))

labelTitle = None
labelUV = None
labelUVIndexText = None
labelPH = None
labelPHinfo = None
labelCl = None
labelClinfo = None
labelAr = None
labelArinfo = None
labelA = None
labelAinfo = None

logo = PhotoImage(file = r"files/4.png").subsample(16,16)
solarLogo = PhotoImage(file = r"files/0.png")
wifiLogo = PhotoImage(file = r"files/w.png").subsample(20,20)

capture = None
imgPUB = None

flagPUB = True
idPUB = 0
flagIndex = 0

window.geometry("320x480")
#window.attributes('-fullscreen','true')
window.resizable(False, False)
window.wm_attributes('-topmost', 1)

window.bind("<Escape>", lambda event:window.destroy())
#window.bind("<Return>", lambda event:changeUV())
window.config(background='black', cursor='none')

def changeValues():
    global values
    global flagPUB
    global idPUB
    global flagIndex

    global wifiLogo

    global labelLocalTime
    global labelTitle
    global labelWifi
    global varUV
    global labelUV
    global labelUVIndexText
    global labelPH
    global labelPHinfo
    global labelCl
    global labelClinfo
    global labelAr
    global labelArinfo
    global labelA
    global labelAinfo
    #values = getValues()
    try:
         values = getValues()
         labelWifi.configure(image = '')
    except Exception as exc:
        print("ERROR: Fail to parsing data")
        print("getValues(): " + str(exc))
        labelWifi.configure(image = wifiLogo)
        labelLocalTime.configure(text = datetime.now().strftime('%Y-%m-%d\n%H:%M'))


    backgroundColor = setUVColor(int(values[1]))

    labelLocalTime.configure(bg = backgroundColor, text = datetime.now().strftime('%Y-%m-%d\n%H:%M'))
    labelWifi.configure(bg = backgroundColor)
    labelTitle.configure(bg = backgroundColor)
    # change UV index value and color
    varUV.set(str(int(values[1])).zfill(2))
    labelUV.configure(image = '', bg = backgroundColor)
    labelUVIndexText.configure(text = setTextInfo(int(values[1])), bg = backgroundColor, font=("Segoe UI", 20), fg = 'white')
    # change pH values
    try:
        labelPH.configure(text = locale.format_string("%.1f", (values[5],1)), bg = backgroundColor)
        #labelPH.configure(bg = backgroundColor)
        labelPHinfo.configure(bg = backgroundColor)
        # change Water Clorium value
        labelCl.configure(text = locale.format_string("%.1f", (values[6],1)), bg = backgroundColor)
        #labelCl.configure(bg = backgroundColor)
        labelClinfo.configure(bg = backgroundColor)
        # change Air Temp value
        labelAr.configure(text = locale.format_string("%.1f", (values[2],1)), bg = backgroundColor)
        labelArinfo.configure(bg = backgroundColor)
        # change Water Temp value
        labelA.configure(text = locale.format_string("%.1f", (values[4],1)), bg = backgroundColor)
        #labelA.configure(bg = backgroundColor)
        labelAinfo.configure(bg = backgroundColor)
    except Exception as exc:
        print("Error: " +str(exc))
        labelPH.configure(bg = backgroundColor)
        #labelPH.configure(bg = backgroundColor)
        labelPHinfo.configure(bg = backgroundColor)
        # change Water Clorium value
        labelCl.configure(bg = backgroundColor)
        #labelCl.configure(bg = backgroundColor)
        labelClinfo.configure(bg = backgroundColor)
        # change Air Temp value
        labelAr.configure(bg = backgroundColor)
        labelArinfo.configure(bg = backgroundColor)
        # change Water Temp value
        labelA.configure(bg = backgroundColor)
        #labelA.configure(bg = backgroundColor)
        labelAinfo.configure(bg = backgroundColor)

    if flagPUB == True:
        if flagIndex == 6:
            flagIndex = 0
            flagPUB = False
        flagIndex += 1
        window.after(3000, changeValues)
    else:
        window.after(5000, changeWaring)

def video():
    global capture
    global labelPUB

    has_frame, frame = capture.read()
    if not has_frame:
        window.after(1, createFrameImage)
        return
    cv2image = cv2.cvtColor(frame, cv2.COLOR_BGR2RGBA)
    img = Image.fromarray(cv2image)
    imgtk = ImageTk.PhotoImage(image=img)
    labelPUB.imgtk = imgtk
    labelPUB.configure(image=imgtk)
    window.after(15, video)

def changePUB():
    global flagPUB
    global framePUB
    global labelPUB
    global windowValue
    global imgPUB
    global idPUB
    global capture

    capture = cv2.VideoCapture(r"files/HotelPub.mp4")

    framePUB = Frame(window)
    labelPUB = Label(framePUB, bg = "black", anchor = CENTER)
    labelPUB.pack(fill = BOTH, expand = True)
    framePUB.pack(fill = BOTH, expand = True)

    frameWarning.destroy()
    flagPUB = True

    window.after(1, video)

def changeWaring():
    global frameWarning
    global labelWarning
    global windowValue
    global imgWarning

    imgWarning = PhotoImage(file = r"files/prec.png")

    frameWarning = Frame(window)
    labelWarning = Label(frameWarning, bg = "black", image = imgWarning, anchor = CENTER)
    labelWarning.pack(fill = BOTH, expand = True)
    frameWarning.pack(fill = BOTH, expand = True)

    windowValue.destroy()

    window.after(7000, changePUB)

def createFrameImage():
    global flagPUB
    global framePUB
    global labelPUB
    global windowValue
    global imgPUB
    global idPUB

    listPUB = [r"files/menu.png"]
    #listPUB = [r"files/menu.png", r"files/spa.png"]
    imgPUB = PhotoImage(file = listPUB[idPUB])

    labelPUB.configure(image = imgPUB)

    if idPUB >= (len(listPUB)-1):
        idPUB = 0
    else:
        idPUB += 1

    window.after(7000, createFrameValues)

def createFrameValues():
    global windowValue
    global labelLocalTime
    global labelTitle
    global labelWifi
    global varUV
    global labelUV
    global labelUVIndexText
    global labelPH
    global labelPHinfo
    global labelCl
    global labelClinfo
    global labelAr
    global labelArinfo
    global labelA
    global labelAinfo
    global framePUB

    windowValue = Frame(window)

    frameTitle = Frame(windowValue)
    labelLocalTime = Label(frameTitle, bg = "blue", text = datetime.now().strftime('%Y-%m-%d\n%H:%M'), font=("Segoe UI", 9, "bold"), fg = "white", width = 3, anchor = "nw", justify=LEFT)
    labelLocalTime.pack(fill = BOTH, side = LEFT, expand = True)
    labelWifi = Label(frameTitle, bg = "blue", anchor = "ne")
    labelWifi.pack(fill = BOTH, side = RIGHT, expand = True)
    labelTitle = Label(frameTitle, bg = "blue", text = "Índice UV\nUV Index", font=("Segoe UI", 16), fg = "white", anchor = "center")
    labelTitle.pack(fill = X, side=TOP, expand = True)

    frameTitle.pack(fill=X)

    frameUV = Frame(windowValue)
    labelUV = Label(frameUV, textvariable = varUV, image = solarLogo, bg = 'blue', font=("Segoe UI", 130, "bold"), fg = "white", highlightthickness = 0, bd = 0)
    labelUV.pack(fill = BOTH, side=TOP)
    labelUVIndexText = Label(frameUV, text = "\nSOLAR KEEPER", bg = 'blue', font=("Segoe UI", 20), fg = "black", anchor = "center")
    labelUVIndexText.pack(fill = BOTH, side=TOP, expand = True)
    frameUV.pack(fill=X)

    frameInfo = Frame(windowValue)

    labelPHinfo = Label(frameInfo, bg = "blue", text = 'pH\nPiscina\n', font=("Segoe UI",9), fg = "white", anchor = "s", width=2)
    labelPHinfo.pack(fill = BOTH, side=LEFT, expand=True)

    labelClinfo = Label(frameInfo, bg = "blue", text = 'Concent.\nde Cloro\n(ORP mV)', font=("Segoe UI", 9), fg = "white", anchor = "s", width=2)
    labelClinfo.pack(fill = BOTH, side=LEFT, expand=True)

    labelAinfo = Label(frameInfo, bg = "blue", text = "Temp Àgua\nWater Temp\n(ºC)", font=("Segoe UI",9), fg = "white", anchor = "s", width=2)
    labelAinfo.pack(fill = BOTH, side=LEFT, expand=True)

    labelArinfo = Label(frameInfo, bg = "blue", text = 'Temp Ar\nAir Temp\n(ºC)', font=("Segoe UI",9),  fg = "white", anchor = "s", width=2)
    labelArinfo.pack(fill = BOTH, side=LEFT, expand=True)

    frameInfo.pack(fill=BOTH, expand = True)

    frameValues = Frame(windowValue)

    labelPH = Label(frameValues, bg = "blue", text = locale.format_string("%.1f",(7.2)), font=("Segoe UI", 16), fg = "white", anchor = "center", width = 2, height=2)
    labelPH.pack(fill = BOTH, side=LEFT, expand=True)

    labelCl = Label(frameValues, bg = "blue", text = locale.format_string("%.1f",(9.3)), font=("Segoe UI", 16), fg = "white", anchor = "center", width = 2, height=2)
    labelCl.pack(fill = BOTH, side=LEFT, expand=True)

    labelA = Label(frameValues, bg = "blue", text = locale.format_string("%.1f",(14.3)), font=("Segoe UI",16), fg = "white", anchor = "center", width = 2, height=2)
    labelA.pack(fill = BOTH, side=LEFT, expand=True)

    labelAr = Label(frameValues, bg = "blue", text = locale.format_string("%.1f", (18)), font=("Segoe UI", 16), fg = "white", anchor = "center", width = 2, height=2)
    labelAr.pack(fill = BOTH, side=LEFT, expand=True)

    frameValues.pack(fill=BOTH, expand = True)

    windowValue.pack(fill=BOTH, expand = True)

    framePUB.destroy()

    changeValues()

windowValue = Frame(window)

frameTitle = Frame(windowValue)
labelLocalTime = Label(frameTitle, bg = "blue", text = datetime.now().strftime('%Y-%m-%d\n%H:%M'), font=("Segoe UI", 9, "bold"), fg = "white", width = 3, anchor = "nw", justify=LEFT)
labelLocalTime.pack(fill = BOTH, side = LEFT, expand = True)
labelWifi = Label(frameTitle, bg = "blue", anchor = "ne")
labelWifi.pack(fill = BOTH, side = RIGHT, expand = True)
labelTitle = Label(frameTitle, bg = "blue", text = "Índice UV\nUV Index", font=("Segoe UI", 16, "bold"), fg = "white", anchor = "center")
labelTitle.pack(fill = X, side=TOP, expand = True)

frameTitle.pack(fill=X)

frameUV = Frame(windowValue)
labelUV = Label(frameUV, textvariable = varUV, image = solarLogo, bg = 'blue', font=("Segoe UI", 130, "bold"), fg = "white", highlightthickness = 0, bd = 0)
labelUV.pack(fill = BOTH, side=TOP)
labelUVIndexText = Label(frameUV, text = "\nSOLAR KEEPER", bg = 'blue', font=("Segoe UI", 20, "bold"), fg = "black", anchor = "center")
labelUVIndexText.pack(fill = BOTH, side=TOP, expand = True)
frameUV.pack(fill=X)

frameInfo = Frame(windowValue)

labelPHinfo = Label(frameInfo, bg = "blue", text = 'pH\nPiscina\n', font=("Segoe UI",9), fg = "white", anchor = "s", width=2)
labelPHinfo.pack(fill = BOTH, side=LEFT, expand=True)

labelClinfo = Label(frameInfo, bg = "blue", text = 'Concent.\nde Cloro\n(ORP mV)', font=("Segoe UI", 9), fg = "white", anchor = "s", width=2)
labelClinfo.pack(fill = BOTH, side=LEFT, expand=True)

labelAinfo = Label(frameInfo, bg = "blue", text = "Temp Àgua\nWater Temp\n(ºC)", font=("Segoe UI",9), fg = "white", anchor = "s", width=2)
labelAinfo.pack(fill = BOTH, side=LEFT, expand=True)

labelArinfo = Label(frameInfo, bg = "blue", text = 'Temp Ar\nAir Temp\n(ºC)', font=("Segoe UI",9),  fg = "white", anchor = "s", width=2)
labelArinfo.pack(fill = BOTH, side=LEFT, expand=True)

frameInfo.pack(fill=BOTH, expand = True)

frameValues = Frame(windowValue)

labelPH = Label(frameValues, bg = "blue", text = locale.format_string("%.1f",(7.2)), font=("Segoe UI", 16), fg = "white", anchor = "center", width = 2, height=2)
labelPH.pack(fill = BOTH, side=LEFT, expand=True)

labelCl = Label(frameValues, bg = "blue", text = locale.format_string("%.1f",(9.8)), font=("Segoe UI", 16), fg = "white", anchor = "center", width = 2, height=2)
labelCl.pack(fill = BOTH, side=LEFT, expand=True)

labelA = Label(frameValues, bg = "blue", text = locale.format_string("%.1f",(14.3)), font=("Segoe UI",16), fg = "white", anchor = "center", width = 2, height=2)
labelA.pack(fill = BOTH, side=LEFT, expand=True)

labelAr = Label(frameValues, bg = "blue", text = locale.format_string("%.1f", (18)), font=("Segoe UI", 16), fg = "white", anchor = "center", width = 2, height=2)
labelAr.pack(fill = BOTH, side=LEFT, expand=True)

frameValues.pack(fill=BOTH, expand = True)

windowValue.pack(fill=BOTH, expand = True)

window.after(1000, changeValues)
window.mainloop()
