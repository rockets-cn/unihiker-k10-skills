---
name: email-sender
description: Send emails via SMTP. Use when the user needs to send emails through QQ Mail, Gmail, Outlook, or any other SMTP-enabled email service. Supports plain text and HTML emails, custom SMTP servers, and app-specific passwords.
---

# Email Sender

Send emails via SMTP with support for multiple email providers.

## When to Use

- Send emails via QQ Mail, Gmail, Outlook, or corporate email
- Use app-specific passwords or authorization codes
- Send plain text or HTML emails
- Bulk email sending

## Quick Start

### QQ Mail (QQ邮箱)

```bash
python scripts/send_email.py \
  --smtp-server smtp.qq.com \
  --smtp-port 587 \
  --sender 1471300@qq.com \
  --password "your-auth-code" \
  --to recipient@example.com \
  --subject "Hello" \
  --body "Email content here"
```

### Gmail

```bash
python scripts/send_email.py \
  --smtp-server smtp.gmail.com \
  --smtp-port 587 \
  --sender your@gmail.com \
  --password "your-app-password" \
  --to recipient@example.com \
  --subject "Hello" \
  --body "Email content here"
```

### Outlook/Hotmail

```bash
python scripts/send_email.py \
  --smtp-server smtp.office365.com \
  --smtp-port 587 \
  --sender your@outlook.com \
  --password "your-password" \
  --to recipient@example.com \
  --subject "Hello" \
  --body "Email content here"
```

## Common SMTP Settings

| Provider | SMTP Server | Port | Password Type |
|----------|-------------|------|---------------|
| QQ Mail | smtp.qq.com | 587 | Authorization Code (授权码) |
| Gmail | smtp.gmail.com | 587 | App Password |
| Outlook | smtp.office365.com | 587 | Account Password |
| 163 Mail | smtp.163.com | 587 | Authorization Code |
| Yahoo | smtp.mail.yahoo.com | 587 | App Password |

## Getting Authorization Codes

### QQ Mail (QQ邮箱)
1. Login to QQ Mail
2. Go to Settings (设置) → Accounts (账户)
3. Find "POP3/IMAP/SMTP" section
4. Enable SMTP service
5. Generate authorization code (授权码)

### Gmail
1. Go to Google Account → Security
2. Enable 2-Step Verification
3. Generate App Password for "Mail"

## Advanced Usage

### Send HTML Email

```bash
python scripts/send_email.py \
  --smtp-server smtp.qq.com \
  --sender 1471300@qq.com \
  --password "your-auth-code" \
  --to recipient@example.com \
  --subject "HTML Email" \
  --body "<h1>Hello</h1><p>This is HTML</p>" \
  --html
```

### Send Email from File

```bash
python scripts/send_email.py \
  --smtp-server smtp.qq.com \
  --sender 1471300@qq.com \
  --password "your-auth-code" \
  --to recipient@example.com \
  --subject "Newsletter" \
  --file email_content.txt
```

## Script Parameters

| Parameter | Required | Default | Description |
|-----------|----------|---------|-------------|
| `--smtp-server` | Yes | - | SMTP server address |
| `--smtp-port` | No | 587 | SMTP server port |
| `--sender` | Yes | - | Sender email address |
| `--password` | Yes | - | Password or authorization code |
| `--to` | Yes | - | Recipient email address |
| `--subject` | Yes | - | Email subject |
| `--body` | Yes* | - | Email body content |
| `--file` | No | - | Read body from file |
| `--html` | No | false | Send as HTML email |

*Required if `--file` is not specified

## Python API

```python
from scripts.send_email import send_email

success, message = send_email(
    smtp_server="smtp.qq.com",
    smtp_port=587,
    sender_email="1471300@qq.com",
    sender_password="your-auth-code",
    recipient="recipient@example.com",
    subject="Hello",
    body="Email content",
    is_html=False
)
```
