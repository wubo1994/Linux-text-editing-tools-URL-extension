Project Title:
	
Linux text editing tools' URL extension.

Motivation:
	
This is a course project of Stony Brook University CSE509 System Security. We got three options and I found this one
to extend all kinds of text editing softwares: (e.g. vim, nano, wc, cat, gedit, etc...) is very interesting, so I ended
up implement it. The idea is to extend these editors so that they can deal with an URL as input, and then read the HTML
page of that URL.

How to build and use:

The software was written under Ubuntu 16.04 environment and the language used is C. Simply type in "make" under the 
project directory and you should see the executable "urlextend". An example using it to extend cat would be:
"./urlextend cat http://www.google.com", then cat will display the HTML content of Google's homepage to you, same
goes for other text editors or tools.

Test:

nano, cat, wc, gedit all works fine without any issue, while vim is able to display the HTML page but upon exit, it
reports stack smashing detected errors, emacs does not work, so far haven't figured out the problem.

Acknowledgement:

The idea of this extension is to use "ptrace" tool to intercept the system calls been made by an application and
modify the system call parameters so that it will load the temporary file that we use to store the HTML page by using "wget".
For the ptrace's usage, I have largely consulted this blog post: https://www.linuxjournal.com/article/6100.
