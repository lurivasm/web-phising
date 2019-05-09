import smtplib
import email
server = smtplib.SMTP('smtp.gmail.com:587')

email_content = """
<html>

<head>

<meta http-equiv="Content-Type" content="text/html; charset=utf-8">

   <title>Amazon</title>
   <style type="text/css">
    a {color: #2D7AF7;}
  body, #header h1, #header h2, p {margin: 0; padding: 0;}
  #main {border: 1px solid #cfcece;}
  img {display: block;}
  #top-message p, #bottom p {color: #3f4042; font-size: 12px; font-family: Arial, Helvetica, sans-serif; }
  #header h1 {color: #000000 !important; font-family: "Lucida Grande", sans-serif; font-size: 24px; margin-bottom: 0!important; padding-bottom: 0; }
  #header p {color: #ffffff !important; font-family: "Lucida Grande", "Lucida Sans", "Lucida Sans Unicode", sans-serif; font-size: 12px;  }
  h5 {margin: 0 0 0.8em 0;}
    h5 {font-size: 18px; color: #FFBB33 !important; font-family: Arial, Helvetica, sans-serif; }
  p {font-size: 12px; color: #8C8686 !important; font-family: "Lucida Grande", "Lucida Sans", "Lucida Sans Unicode", sans-serif; line-height: 1.5;}
  #negro {color: #000000 !important; font-size: 14px;}
   </style>
</head>

<body>


<table width="100%" cellpadding="0" cellspacing="0" bgcolor="e4e4e4"><tr><td>
<table id="top-message" cellpadding="20" cellspacing="0" width="600" align="center">
    <tr>
      <td align="center">
        <p><a href="#">Ir al navegador</a></p>
      </td>
    </tr>
  </table>

<table id="main" width="600" align="center" cellpadding="0" cellspacing="15" bgcolor="ffffff">
    <tr>
      <td>
        <table id="header" cellpadding="20" cellspacing="0" align="center" bgcolor="8fb3e9">
          <tr>
            <td width="800" align="center"  bgcolor="#FFBB33"><h1>Amazon te quiere invitar!</h1></td>
          </tr>
        </table>
      </td>
    </tr>

    <tr>
      <td>
        <table id="content-3" cellpadding="0" cellspacing="0" align="center">
          <tr>
              <td width="250" valign="top" bgcolor="d0d0d0" style="padding:5px;">
              <img src="https://cdn.computerhoy.com/sites/navi.axelspringer.es/public/styles/480/public/media/image/2019/03/amazon-primavera.jpg?itok=d-gQG2kO" width="270" height="180"  />
            </td>
              <td width="15"></td>
            <td width="200" valign="top">
                <h5>Obtén tu código ya!</h5>
                <p id="negro">Para agradecerte el tiempo que llevas con nosotros te queremos invitar a un <b> 25% de descuento </b> en tu próxima compra. Accede ahora y desbloquea tu regalo con el código promocional <b>27gK9Hx</b>.</p>
            </td>
          </tr>
        </table>
      </td>
    </tr>
    <tr>
      <td align="center">
         <a href="http://localhost:9990"><img src="https://jarcakids.com/custom_files/boton_iniciar_sesion.png" width="130" height="40"></a>
      </td>
    </tr>
  </table>

  <table id="bottom" cellpadding="20" cellspacing="0" width="600" align="center">
    <tr>
      <td align="center">
        <p>Promoción válida para compras inferiores a 150$</p>
        <p>Válida hasta el 16/05/2019</p>
        <p>Código no acumulable a otros descuentos</p>
        <p><a href="#">Cancelar suscripción</a> | <a href="#">Más información</a> | <a href="#">Ir al navegador</a></p>
      </td>
    </tr>
  </table><!-- top message -->
</td></tr></table><!-- wrapper -->

</body>
</html>

"""

msg = email.message.Message()
msg['Subject'] = 'Amazon te regala un descuento'


msg['From'] = 'amazoncompanyespana@gmail.com'
msg['To'] = 'lucia_rm98@hotmail.com'
password = "hola-123"
msg.add_header('Content-Type', 'text/html')
msg.set_payload(email_content)

s = smtplib.SMTP('smtp.gmail.com: 587')
s.starttls()

# Login Credentials for sending the mail
s.login(msg['From'], password)

s.sendmail(msg['From'], [msg['To']], msg.as_string().encode('utf-8'))
