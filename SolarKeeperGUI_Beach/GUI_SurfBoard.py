#!/usr/bin/env python3
# coding=utf-8

from backend import *

#values = [0:uvValueA 1:uVindex 2:tempAr 3:HumAr 4:tempAgua 5:pH 6:ConcentrCloro]
values = [0, 0, 22, 0, 14.3, 7.3, 9.8]

window = Tk()

varUV = StringVar()
varUV.set(str().zfill(2))

logo = PhotoImage(file = r"./files/3.png").subsample(16,16)
solarLogo = PhotoImage(file = r"./files/0.png")
wifiLogo = PhotoImage(file = r"./files/w.png").subsample(20,20)

#window.geometry("320x480")
#window.wm_attributes('-fullscreen','true')
window.wm_attributes('-zoomed','true')
#window.resizable(False, False)
window.wm_attributes('-topmost', 1)

window.bind("<Escape>", lambda event:window.destroy())
#window.bind("<Return>", lambda event:changeUV())
window.config(background='blue', cursor='none')

def changeValues():
    global values
    global wifiLogo

    global labelLocalTime
    global labelTitle
    global labelWifi
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

    # labelUVInfo.configure(bg = setUVColor(int(values[1])))
    # change UV index value and color
    varUV.set(str(int(values[1])).zfill(2))
    labelUV.configure(image = '', bg = backgroundColor)
    labelUVIndexText.configure(text = setTextInfo(int(values[1])), bg = backgroundColor, font=("Segoe UI", 20), fg = 'white')
    # change pH values
    labelTide.configure(bg = backgroundColor)
    labelTideinfo.configure(bg = backgroundColor)
    # change Water Clorium value
    labelQual.configure(bg = backgroundColor)
    labelQualinfo.configure(bg = backgroundColor)
    # change Air Temp value
    labelAr.configure(text = locale.format_string("%.1fºC", (values[2],1)), bg = backgroundColor)
    labelArinfo.configure(bg = setUVColor(int(values[1])))
    # change Water Temp value
    #labelA.configure(text = locale.format_string("%.1fºC", (values[4],1)), bg = setUVColor(int(values[1])))
    labelA.configure(bg = backgroundColor)
    labelAinfo.configure(bg = backgroundColor)

    window.after(3000, changeValues)

frameTitle = Frame(window)

labelLocalTime = Label(frameTitle, bg = "blue", text = datetime.now().strftime('%Y-%m-%d\n%H:%M'), font=("Segoe UI", 9, "bold"), fg = "white", width = 3, anchor = "nw", justify=LEFT)
labelLocalTime.pack(fill = 'both', side = LEFT, expand = True)
labelWifi = Label(frameTitle, bg = "blue", anchor = "ne")
labelWifi.pack(fill = 'both', side = RIGHT, expand = True)
labelTitle = Label(frameTitle, bg = "blue", text = "Índice UV\nUV Index", font=("Segoe UI", 16, "bold"), fg = "white", anchor = "center")
labelTitle.pack(fill = 'x', side=TOP, expand = True)
frameTitle.pack(fill='x')

frameUV = Frame(window)

# labelUVInfo = Label(frameUV, bg = "blue", text = "Índice UV\nUV Index", font=("Segoe UI", 18), fg = "white", anchor = "center")
# labelUVInfo.pack(fill = BOTH, side=TOP, expand = True)
labelUV = Label(frameUV, textvariable = varUV, image = solarLogo, bg = 'blue', font=("Segoe UI", 130, "bold"), fg = "white", highlightthickness = 0, bd = 0)
labelUV.pack(fill = 'both', side=TOP)
labelUVIndexText = Label(frameUV, text = "\nSOLAR KEEPER", bg = 'blue', font=("Segoe UI", 20), fg = "black", anchor = "center")
labelUVIndexText.pack(fill = 'both', side=TOP, expand = True)
frameUV.pack(fill='x')

frameInfo = Frame(window)

labelTideinfo = Label(frameInfo, bg = "blue", text = 'Próximas\nMarés', font=("Segoe UI",9), fg = "white", anchor = "s", width=2)
labelTideinfo.pack(fill = BOTH, side=LEFT, expand=True)

labelQualinfo = Label(frameInfo, bg = "blue", text = 'Qualidade\nda praia', font=("Segoe UI", 9), fg = "white", anchor = "s", width=2)
labelQualinfo.pack(fill = BOTH, side=LEFT, expand=True)

labelAinfo = Label(frameInfo, bg = "blue", text = "Temp Àgua\nWater Temp", font=("Segoe UI",9), fg = "white", anchor = "s", width=2)
labelAinfo.pack(fill = BOTH, side=LEFT, expand=True)

labelArinfo = Label(frameInfo, bg = "blue", text = 'Temp Ar\nAir Temp', font=("Segoe UI",9),  fg = "white", anchor = "s", width=2)
labelArinfo.pack(fill = BOTH, side=LEFT, expand=True)

frameInfo.pack(fill=BOTH, expand = True)

frameValues = Frame(window)

labelTide = Label(frameValues, bg = "blue", text = "14:50\n20:38", font=("Segoe UI", 16), fg = "white", anchor = "center", width = 2, height=2)
labelTide.pack(fill = 'both', side=LEFT, expand=True)

labelQual = Label(frameValues, bg = "blue", image = logo, font=("Segoe UI", 16), fg = "white", anchor = "center", width = 2, height=2)
labelQual.pack(fill = 'both', side=LEFT, expand=True)

labelA = Label(frameValues, bg = "blue", text = locale.format_string("%.1fºC",(14.3)), font=("Segoe UI",16), fg = "white", anchor = "center", width = 2, height=2)
labelA.pack(fill = 'both', side=LEFT, expand=True)

labelAr = Label(frameValues, bg = "blue", text = locale.format_string("%.1fºC", (18)), font=("Segoe UI", 16), fg = "white", anchor = "center", width = 2, height=2)
labelAr.pack(fill = 'both', side=LEFT, expand=True)

frameValues.pack(fill='both', expand = True)


window.after(5000, changeValues)
window.mainloop()
