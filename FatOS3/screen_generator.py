from graphics import *
import os
from winsound import *

win = GraphWin("FatOS Graphics Creator", 600, 800)
colors = ["black", "white", "red", "green", "blue", "yellow", "navy", "maroon"]

table = []
for i in range(30):
    row = []
    for j in range(40):
        row.append(1)
    table.append(row)

r = Rectangle(Point(0, 0), Point(600, 800))
r.setFill(colors[1])
r.draw(win)

for i in range(1, 30):
    l = Line(Point(i * 20, 0), Point(i * 20, 800))
    l.draw(win)
for i in range(1, 40):
    l = Line(Point(0, i * 20), Point(600, i * 20))
    l.draw(win)

def roundDown(n):
    return int(n - (n % 20))

u = input("Load file? ")
if u != 'n':
    try:
        path = "./SD_contents/apps/" + u + '/' + u + ".txt"
        load = open(path, 'r')
        read = load.readlines()
        readX = 0
        readY = 0

        for i in range(len(read)):
            row = read[i].split(' ')
            for j in range(len(row)):
                table[readX][readY] = int(row[j])
                chrCode = int(row[j]) >> 8
                color1 = (int(row[j]) >> 4) & 15
                color2 = int(row[j]) & 15
                r = Rectangle(Point(readX * 20, readY * 20), Point(readX * 20 + 20, readY * 20 + 20))
                r.setFill(colors[color2])
                r.draw(win)
                c = Text(Point(readX * 20 + 10, readY * 20 + 10), str(chrCode))
                c.setFill(colors[color1])
                c.draw(win)
                readX = readX + 1
            readX = 0
            readY = readY + 1
    except IOError:
        print("File doesn't exist")
            

while True:
    pt = win.checkMouse()
    if pt != None:
        try:
            Beep(800, 200)
            x1 = int(roundDown(pt.getX()) / 20)
            y1 = int(roundDown(pt.getY()) / 20)
            pt = win.getMouse()
            Beep(800, 200)
            x2 = int(roundDown(pt.getX()) / 20)
            y2 = int(roundDown(pt.getY()) / 20)
            key = win.getKey()
            Beep(800, 200)
            chrCode = int(key) * 10
            key = win.getKey()
            Beep(800, 200)
            chrCode = chrCode + int(key)
            key = win.getKey()
            Beep(800, 200)
            color1 = int(key)
            key = win.getKey()
            Beep(800, 200)
            color2 = int(key)
            for y in range(y1, y2 + 1):
                for x in range(x1, x2 + 1):
                    table[x][y] = (chrCode << 8) + (color1 << 4) + (color2)
                    r = Rectangle(Point(x * 20, y * 20), Point(x * 20 + 20, y * 20 + 20))
                    r.setFill(colors[color2])
                    r.draw(win)
                    c = Text(Point(x * 20 + 10, y * 20 + 10), str(chrCode))
                    c.setFill(colors[color1])
                    c.draw(win)
        except:
            print("Invalid code (must be a number)")
    key = win.checkKey()
    if key == 's':
        try:
            name = input("File name? ")
            path = "./SD_contents/apps/" + name + '/' + name + ".txt"
            os.makedirs(os.path.dirname(path), exist_ok = True)
            file = open(path, 'w')
            for y in range(40):
                for x in range(30):
                    file.write(str(table[x][y]))
                    if x != 29:
                        file.write(' ')
                file.write('\n')
            file.close()
            print("Saved")
        except Exception as e:
            print("Error: " + str(e))
            try:
                file.close()
            except:
                pass
        
