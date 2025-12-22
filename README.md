# File Integrity Checker - Platinum Edition 🛡️

![Version](https://img.shields.io/badge/version-Platinum-blueviolet)
![Language](https://img.shields.io/badge/language-C-00599C)
![GUI](https://img.shields.io/badge/GUI-GTK%2B3-green)
![Database](https://img.shields.io/badge/database-SQLite3-blue)

A professional-grade system security and monitoring tool designed to detect unauthorized file modifications. Built from scratch in **C**, this application uses cryptographic hashing to create "digital fingerprints" of files, allowing users to verify data integrity over time.

---

## 🚀 Key Features

### 🔒 1. Cryptographic Verification
- Uses the **SHA-256** algorithm (via OpenSSL) to generate unique 64-character hash signatures for every file.
- Detects even single-bit modifications in files of any size.

### 📂 2. Multithreaded Directory Scanner
- Scans entire directory trees recursively without freezing the user interface.
- Powered by **GThread** for non-blocking performance.
- Capable of processing thousands of files in background threads.

### 📊 3. Visual Analytics & Reporting
- **Global Statistics:** A dynamic Pie Chart (using **Cairo**) showing the ratio of Passed vs. Failed integrity checks.
- **Per-File Timeline:** Double-click any file in the history log to see a dedicated **Bar Chart** of that specific file's modification history.

### 💾 4. Persistent Database
- Automatically saves every scan result to a local **SQLite** database (`integrity_history.db`).
- History survives application restarts, allowing for long-term monitoring.

### 🛠️ 5. Integrated File Tools
- **File Management:** Create new files and folders directly from the scanner interface.
- **Sorting:** Sort directory results by File Name, Hash, or **File Extension**.

### 🎨 6. Modern UI/UX
- Custom **High-Contrast Dark Theme** for professional environments.
- Optimized color coding: **Green** for verified matches, **Red** for mismatches.

---

## 🛠️ Technology Stack

| Component | Technology | Description |
| :--- | :--- | :--- |
| **Core Logic** | **C (C99)** | Low-level system access and memory management. |
| **Interface** | **GTK+ 3** | Cross-platform GUI framework. |
| **Security** | **OpenSSL** | Industry-standard cryptographic library. |
| **Database** | **SQLite 3** | Serverless, self-contained SQL database engine. |
| **Graphics** | **Cairo** | 2D vector graphics for drawing charts. |
| **Concurrency** | **GThread** | POSIX-compliant threading for C. |

---

## ⚙️ Installation & Setup

### Prerequisites (Windows)
This project is built using **MSYS2 MinGW x64**. You need the following packages installed:

```bash
pacman -S mingw-w64-x86_64-gtk3
pacman -S mingw-w64-x86_64-openssl
pacman -S mingw-w64-x86_64-sqlite3
pacman -S mingw-w64-x86_64-toolchain
