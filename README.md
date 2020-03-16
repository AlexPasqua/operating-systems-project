# Practice project for the course "Operating Systems" @ UniVR.
## By *Alex Pasquali*

### *(Read "consegna-pdf" (in Italian) for more explenation -> <u>NOTE: the project's language is Italian</u>)* <br><br>

<font size=4>**NOTE: use Linux Bash or Windows' Linux subsystem emulator**</font>

## Step 1:

<font size=4>
	1. **Open the bash and enter *system-call* directory.** <br>
	2. **With one bash window, open *clientReq-server* directory. Type *make*** <br>
	3. **With a second bash window, open *clientExec* directory. Type *make*** <br>
	4. **Open a third bash window and go into *clientReq-server* directory** <br>
</font>
<br>

## Step 2:
<font size=4>
	1. **With the first bash** (in *clientReq-server* directory) **execute the server process:** <font face="consolas">&ensp;./server</font> <br>
	2. **With the third bash** (in *clientReq-server* directory) **exectute *"clientReq"*:** <font face="consolas">&ensp;./clientReq</font> <br>
</font>

<font size=4><u> Now everything is ready to begin using the program</u></font><br><br>

## Step 3:
<font size=4>
	1. **Insert a userid with *clientReq*** <br>
	2. **Insert the name of the desired service** ("stampa", "salva", "invia") <br>
	3. **WIth the bash in *clientExec* directory execute *"clientExec":*** <font face="consolas">&ensp;./clientExec *userid server_key args* </font><br>
		&ensp;&ensp;&ensp;&ensp;&ensp;
		note: *userid* is the one chosen by the user (in *clientReq*)<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;
		*server_key* is the code returned by the server process after entering data with *clientReq*<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;
		*args*: see below <br>

*args*, depending on the service chosen, is... <br>
1. service *stampa*: what you want to print <br>
2. service *salva*: the name of the destination file followed by what you want to write in it <br>
3. service *invia*: id key of a message queue already existing in the system followed by what you want to send in it <br>
</font><br>

<font size=6> **To kill:** </font><br>
<font size=4>
	**NOTE:** the *server* is only sensitive to **SIGTERM**. So to kill it it's necessary to throw:
	<font face="consolas">
		killall server <br>
	</font>
	<u>PAY ATTENTION NOT TO KILL THE WRONG PROCESS!!!</u>
</font>
<br><br><br>


<font size=4><u> **In case of problems</u>:**</font>
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