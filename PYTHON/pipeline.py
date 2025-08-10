import os
import re
import csv
from datetime import datetime
from concurrent.futures import ProcessPoolExecutor
from collections import defaultdict
import smtplib
from email.mime.text import MIMEText

# === CONFIG ===
LOG_ROOT = "/central_logs"
REPORT_DIR = f"/reports/{datetime.now().strftime('%Y-%m-%d')}"
ALERT_CSV = os.path.join(REPORT_DIR, "alerts.csv")
SUMMARY_CSV = os.path.join(REPORT_DIR, "report.csv")
SMTP_SERVER = "smtp.example.com"
EMAIL_FROM = "alerts@example.com"
EMAIL_TO = "ops-team@example.com"

ERROR_PATTERNS = {
    "OOM": re.compile(r"Out of memory"),
    "Segfault": re.compile(r"segfault at"),
    "Kernel Panic": re.compile(r"kernel panic", re.IGNORECASE),
    "Disk Error": re.compile(r"I/O error|ext[34]_fs error", re.IGNORECASE),
}

# === UTILITIES ===
def parse_log_line(line):
    match = re.match(r'^(\w{3} \d+ \d{2}:\d{2}:\d{2}) (\S+) (\S+): (.*)', line)
    if match:
        time_str, host, source, msg = match.groups()
        timestamp = datetime.strptime(time_str, "%b %d %H:%M:%S")
        return {"timestamp": timestamp, "host": host, "source": source, "message": msg}
    return None

def match_patterns(message):
    return [name for name, pattern in ERROR_PATTERNS.items() if pattern.search(message)]

def scan_log_file(path, host):
    alerts = []
    stats = defaultdict(int)
    try:
        with open(path) as f:
            for line in f:
                entry = parse_log_line(line)
                if not entry:
                    continue
                entry['host'] = host
                matches = match_patterns(entry['message'])
                if matches:
                    alerts.append((entry, matches))
                    for m in matches:
                        stats[m] += 1
    except Exception as e:
        print(f"Error reading {path}: {e}")
    return alerts, stats

def find_crash_cores(host_dir):
    crash_dir = os.path.join(host_dir, "crash")
    cores = []
    if os.path.isdir(crash_dir):
        for fname in os.listdir(crash_dir):
            if fname.startswith("core"):
                cores.append(fname)
    return cores

def write_alerts_csv(alerts):
    with open(ALERT_CSV, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(["Timestamp", "Host", "Source", "Message", "Matched Patterns"])
        for entry, patterns in alerts:
            writer.writerow([
                entry["timestamp"], entry["host"], entry["source"],
                entry["message"], ";".join(patterns)
            ])

def write_summary_csv(stats_by_host, crash_summary):
    with open(SUMMARY_CSV, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(["Host", "Pattern", "Count", "Core Dumps"])
        for host, patterns in stats_by_host.items():
            crash_count = len(crash_summary.get(host, []))
            for pattern, count in patterns.items():
                writer.writerow([host, pattern, count, crash_count])


def send_email_alert(subject, body):
    msg = MIMEText(body)
    msg['Subject'] = subject
    msg['From'] = EMAIL_FROM
    msg['To'] = EMAIL_TO
    with smtplib.SMTP(SMTP_SERVER) as server:
        server.send_message(msg)

# === MAIN PIPELINE ===
def main():
    os.makedirs(REPORT_DIR, exist_ok=True)
    hosts = [d for d in os.listdir(LOG_ROOT) if os.path.isdir(os.path.join(LOG_ROOT, d))]

    all_alerts = []
    stats_by_host = {}
    crash_summary = {}

    with ProcessPoolExecutor() as executor:
        futures = {}
        for host in hosts:
            log_path = os.path.join(LOG_ROOT, host, f"messages.{datetime.now().strftime('%Y-%m-%d')}")
            futures[executor.submit(scan_log_file, log_path, host)] = host
            crash_summary[host] = find_crash_cores(os.path.join(LOG_ROOT, host))

        for future in futures:
            host = futures[future]
            alerts, stats = future.result()
            all_alerts.extend(alerts)
            stats_by_host[host] = stats

    write_alerts_csv(all_alerts)
    write_summary_csv(stats_by_host, crash_summary)

    if all_alerts:
        send_email_alert("[ALERT] Issues found in nightly logs", f"{len(all_alerts)} alerts detected. See CSV for details.")

if __name__ == "__main__":
    main()

