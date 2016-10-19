#!/usr/bin/env python

import serial
import time
import random
import math
import sys
from cStringIO import StringIO
#from io import StringIO
import socket

#try:
ser = serial.Serial(
    port='/dev/ttyACM0',
    baudrate=38400
)
#except Exception,e:
#   ser = None
#   udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#   udp.bind(('', 7654))

#ser.open()
#ser.isOpen()

#print ser.BAUDRATES
#xprint ser.writable()

time.sleep(1)

def buffer(f):
    return [[f(x,y) for y in range(112)] for x in range(20)]

def update(buf,f):
    for x in range(20):
        for y in range(112):
            buf[x][y] = f(x,y)

def plot(out,x,y,v):
    out.write(chr(x+128*v) + chr(y))

def plotall(out, old, new):
    for x in range(20):
        for y in range(112):
            if new[x][y] != old[x][y]:
                plot(out,x,y,new[x][y])

def display(old, new):
    if ser:
        plotall(ser, old, new)
        ser.flush()
    else:
        out = StringIO()
        for x in range(20):
            b = 0
            v = 0
            for y in range(112):
                v |= new[x][y] << b
                b += 1
                if b == 8:
                    out.write(chr(v))
                    b = 0
                    v = 0
        data = out.getvalue()
        address = ("10.0.2.81", 4334)
        udp.sendto(data, address)
        udp.sendto(data, address)
        udp.sendto(data, address)
        time.sleep(0.03)

random.seed()

A = buffer(lambda x,y : 1)
B = buffer(lambda x,y : 0)
display(A,B)

def GameOfLife():
    global A,B
    update(A, lambda x,y : random.randint(0,1))
    display(B,A)

    while True:
        for x in range(20):
            for y in range(112):
                n = 0
                for dx in range(-1,2):
                    for dy in range(-1,2):
                        n += A[(x+dx) % 20][(y+dy) % 112]
                c = A[x][y]
                d = int(n >= 3 and n <= 3+c)
                B[x][y] = d
        display(A,B)
        A,B = B,A

def Metaballs():
    global A,B
    p = [(0.0,0.0),(0.0,0.0),(0.0,0.0),(0.0,0.0),(0.0,0.0)]
    t = 0
    while True:
        for i in range(len(p)):
            p[i] = (10.0+10.0*math.sin(t*(0.035+i*0.012)), 56.0+56.0*math.sin(t*(0.00413425+i*0.0023521)))

        def meta(x,y):
            s = 0.0
            for i in range(len(p)):
                dx = x - p[i][0]
                dy = y - p[i][1]
                d = math.exp(-0.01*(dx*dx + dy*dy))
                s += d
            return int(s > 0.5 and s < 0.8)

        update(A, meta)
        display(B,A)
        A,B = B,A
        t += 1

def Cube():
    global A,B
    corners = [(float(i & 1)-0.5, float((i >> 1) & 1)-0.5, float((i >> 2) & 1)-0.5) for i in range(8)]
    #print corners
    #sys.stdout.flush()
    faces = [[0,1,3,2], [0,2,6,4], [0,4,5,1], [7,5,4,6], [7,3,1,5], [7,6,2,3]]
    angles = [0.0, 0.0, 0.0]
    t = 0
    while True:
        rcorners = []
        for c in corners:
            c = list(c)
            for i,a in enumerate(angles):
                ca, sa = math.cos(a), math.sin(a)
                c0, c1 = c[i-2], c[i-1]
                c[i-2] = c0*ca - c1*sa
                c[i-1] = c0*sa + c1*ca
            x,y,z = tuple(c)
            zfac = 100.0/(z+3.0+math.sin(t*0.01))
            rcorners.append((x*zfac+10.0, y*zfac+56.0))
        #print rcorners
        #sys.stdout.flush()
        for x in range(20):
            for y in range(112):
                col = -1
                for fi,f in enumerate(faces):
                    inside = True
                    for fc in range(-1,3):
                        rc0, rc1 = rcorners[f[fc]], rcorners[f[fc+1]]
                        v0 = (rc0[0]-x, rc0[1]-y)
                        v1 = (rc1[0]-rc0[0], rc1[1]-rc0[1])
                        a = v0[0]*v1[1] - v0[1]*v1[0]
                        if a < 0:
                            inside = False
                            break
                    if inside:
                        col = fi
                        break
                if col == -1:
                    p = ((x+y) % 6) / 3
                elif (col % 3) == 2:
                    p = (x^y) & 1
                else:
                    p = col % 3
                A[x][y] = p
        display(B,A)
        A,B = B,A
        angles[0] += 0.0137
        angles[1] += 0.03629
        angles[2] += 0.02871
        t += 1


#GameOfLife()
Metaballs()
#Cube()

ser.close()
