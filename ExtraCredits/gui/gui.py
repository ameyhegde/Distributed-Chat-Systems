#!/usr/bin/python
import time
import subprocess

from Tkinter import *

args = ("./dchat", "bob")
popen = subprocess.Popen(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
#print popen.stdout.readline()
popen.communicate("hey there")

def uiFunc(name):

	root = Tk()
	root.wm_title(name)
	#chatMsg = Label(root, text = "Full chat will come here")
	#chatMsg.pack()

	chatFrame = Frame(root)
	chatFrame.pack()
	#chatFrame.configure(background='white')

	typeFrame = Frame(root)
	typeFrame.pack(side=BOTTOM)

	

	chatMsg = Label(chatFrame, text = "ready", justify=LEFT, bg="white")
	chatMsg.pack()

	E1 = Entry(typeFrame)
	E1.pack(side = LEFT)

	def callback():
    		str1 = E1.get()
		
		

	button1 = Button(typeFrame, text = "Send", bg = "green", fg = "white", command=callback)

	button1.pack()
	
	
	#print output

	var = StringVar()
	
	#var.set(output)

	
	#for i in range(100):
        '''
    	while True:
		line = popen.stdout.read()
    		var.set(line)
    		root.update_idletasks()
    		root.update()
	'''
	root.mainloop()




#uiFunc("bob")


