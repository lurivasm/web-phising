import smtplib
import email
server = smtplib.SMTP('smtp.gmail.com:587')

email_content = """type here the content, html code"""

msg = email.message.Message()
msg['Subject'] = 'type here the subject'


msg['From'] = 'email from'
msg['To'] = 'email dest'
password = "password email from"
msg.add_header('Content-Type', 'text/html')
msg.set_payload(email_content)

s = smtplib.SMTP('smtp.gmail.com: 587')
s.starttls()

# Login Credentials for sending the mail
s.login(msg['From'], password)

s.sendmail(msg['From'], [msg['To']], msg.as_string().encode('utf-8'))
