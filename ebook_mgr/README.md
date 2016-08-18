document
========

Personal e-Document Management Utilities

About to write a Web-based application to maintain
the large amount of electronic documents on my hard drive.

There may first be a PHP program, and then some cli tools,
and may be more.

# *** nothing modified.


# cgi prog to fetch a specified page of pdf file
#!/bin/bash
if [ -z $1 ] || [ -z $2 ];then
    echo "Usage: $0 PAGE FILE"
    echo "      PAGE    the page you need from specified PDF file"
    echo "      FILE    the PDF file you want to operate on"
else
    pdftoppm -f $2 -l $2 $1 | ppmtojpeg
fi