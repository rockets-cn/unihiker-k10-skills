import smtplib
import sys
import argparse
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from email.header import Header

def send_email(smtp_server, smtp_port, sender_email, sender_password, recipient, subject, body, is_html=False):
    """Send email using SMTP"""
    msg = MIMEMultipart()
    msg['From'] = sender_email
    msg['To'] = recipient
    msg['Subject'] = Header(subject, 'utf-8')
    
    content_type = 'html' if is_html else 'plain'
    msg.attach(MIMEText(body, content_type, 'utf-8'))
    
    try:
        with smtplib.SMTP(smtp_server, smtp_port) as server:
            server.starttls()
            server.login(sender_email, sender_password)
            server.sendmail(sender_email, recipient, msg.as_string())
        return True, "Email sent successfully"
    except Exception as e:
        return False, str(e)

def main():
    parser = argparse.ArgumentParser(description='Send email via SMTP')
    parser.add_argument('--smtp-server', required=True, help='SMTP server address')
    parser.add_argument('--smtp-port', type=int, default=587, help='SMTP server port (default: 587)')
    parser.add_argument('--sender', required=True, help='Sender email address')
    parser.add_argument('--password', required=True, help='Sender email password or app-specific password')
    parser.add_argument('--to', required=True, help='Recipient email address')
    parser.add_argument('--subject', required=True, help='Email subject')
    parser.add_argument('--body', required=True, help='Email body content')
    parser.add_argument('--html', action='store_true', help='Send as HTML email')
    parser.add_argument('--file', help='Read body from file')
    
    args = parser.parse_args()
    
    # Read body from file if specified
    body = args.body
    if args.file:
        try:
            with open(args.file, 'r', encoding='utf-8') as f:
                body = f.read()
        except Exception as e:
            print(f"[ERROR] Failed to read file: {e}")
            sys.exit(1)
    
    success, message = send_email(
        args.smtp_server,
        args.smtp_port,
        args.sender,
        args.password,
        args.to,
        args.subject,
        body,
        args.html
    )
    
    if success:
        print(f"[OK] {message}")
        sys.exit(0)
    else:
        print(f"[ERROR] {message}")
        sys.exit(1)

if __name__ == '__main__':
    main()
