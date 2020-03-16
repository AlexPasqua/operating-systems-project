# Practice project for the course "Operating Systems" @ UniVR.
## By *Alex Pasquali*

### *(Read "consegna-pdf" (in Italian) for more explenation -> <u>NOTE: the project's language is Italian</u>)* <br><br>

<font size=4>**NOTE: use Linux Bash or Windows' Linux subsystem emulator**</font>


## Step 1:
<font size=4>
	<b>
		1. Open the bash and enter <i>system-call</i> directory.<br>
		2. With one bash window, open <i>clientReq-server</i> directory. Type <i>make</i><br>
		3. With a second bash window, open <i>clientExec</i> directory. Type <i>make</i><br>
		4. Open a third bash window and go into <i>clientReq-server</i> directory<br>
	</b>
</font>
<br>


## Step 2:
<font size=4>
1. <b>With the first bash </b>(in <i>clientReq-server</i> directory)<b> execute the server process:</b> <font face="consolas">&ensp;./server</font> <br>
2. <b>With the third bash </b>(in <i>clientReq-server</i> directory)<b> exectute <i>"clientReq"</i>:</b> <font face="consolas">&ensp;./clientReq</font> <br>
</font>

<font size=4><u> Now everything is ready to begin using the program</u></font><br><br>


## Step 3:
<font size=4>
	<b>
		1. Insert a userid with <i>clientReq</i> <br>
		2. Insert the name of the desired service
	</b>
	("stampa", "salva", "invia") <br>
	<b>3.  WIth the bash in <i>clientExec<i> directory execute <i>"clientExec"</i>:</b>
	<font face="consolas">&ensp;./clientExec <i>userid server_key args</i> </font><br>
	&ensp;&ensp;&ensp;&ensp;&ensp;
	note: <i>userid</i> is the one chosen by the user (in <i>clientReq</i>)<br>
	&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;
	<i>server_key</i> is the code returned by the server process after entering data with <i>clientReq</i><br>
	&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;
	<i>args</i>: see below <br>

<i>args</i>, depending on the service chosen, is... <br>
1. service <i>stampa</i>: what you want to print <br>
2. service <i>salva</i>: the name of the destination file followed by what you want to write in it <br>
3. service <i>invia</i>: id key of a message queue already existing in the system followed by what you want to send in it <br>
</font><br>

</i></i>
<font size=6> <b>To kill:</b> </font><br>
<font size=4>
	<b>NOTE:</b> the <i>server</i> is only sensitive to <b>SIGTERM</b>. So to kill it it's necessary to throw:
	<font face="consolas">killall server </font><br>
	<u>PAY ATTENTION NOT TO KILL THE WRONG PROCESS!!!</u>
</font>
<br><br><br>


<font size=4><u><b>In case of problems</b></u>:</font>
<font size=3>
	execute the following<br>
	<font face="consolas">
		&ensp;&ensp;ipcrm --semaphore-id X <br>
		&ensp;&ensp;ipcrm --shmem-id X <br>
		&ensp;&ensp;rm /tmp/FIFOSERVER <br>
		&ensp;&ensp;rm /tmp/FIFOCLIENT <br>
	</font>
	(where X stands for the id of the IPC. Try starting from 0 and repeting the same command until it says
	<font face="consolas">&ensp;ipcrm: invalid id (X)</font><br>
	<u>NOTE</u>: this will delete some IPCs in your system. If you're aware of some needed IPC that you don't want to delete, please be sure not to insert its id key.<br><br>
	Now everything should work properly, otherwise open a new issue (or check existing ones) if you pleased. <br> Thank you.
</font>
