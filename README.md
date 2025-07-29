<p align="center">
  <img src="logo_watersn.png" alt="WaterSN Logo" width="180"/>
</p>
<h1 align="center">WaterSN</h1>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue?logo=c%2B%2B&logoColor=white" alt="C++17"/>
  <img src="https://img.shields.io/badge/Qt-6.9.1-brightgreen?logo=qt&logoColor=white" alt="Qt 6.9.1"/>
  <img src="https://img.shields.io/badge/MySQL-Database-orange?logo=mysql&logoColor=white" alt="MySQL"/>
  <img src="https://img.shields.io/badge/CMake-Build-lightgrey?logo=cmake&logoColor=white" alt="CMake"/>
  <img src="https://img.shields.io/badge/Linux-Supported-important?logo=linux&logoColor=white" alt="Linux"/>
</p>

## Overview

**WaterSN** is a modern, modular desktop application for water billing and management, designed for water utilities and local authorities. It provides a secure, user-friendly interface for managing clients, billing, meter readings, and more.

## Features

- **Authentication**: Secure login, password reset by email, default admin user
- **User Management**: Roles, permissions, and user CRUD
- **Client Management**: Add, edit, and search clients
- **Meter Management**: Register and track water meters
- **Billing**: Generate, view, and export water bills
- **Payments**: Record and track payments
- **Statistics & Charts**: Visualize consumption and revenue
- **Alerts**: System notifications and reminders
- **Modern UI/UX**: Responsive, branded interface with logo
- **MySQL Database**: Robust, scalable data storage
- **Native Email Sending**: SMTP/SSL for password reset (no external libs)

## Technologies Used

<p>
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/cplusplus/cplusplus-original.svg" width="40" title="C++"/>
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/qt/qt-original.svg" width="40" title="Qt"/>
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/mysql/mysql-original.svg" width="40" title="MySQL"/>
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/cmake/cmake-original.svg" width="40" title="CMake"/>
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/linux/linux-original.svg" width="40" title="Linux"/>
</p>

## Project Structure

- `main.cpp` — Application entry point
- `watersn.cpp/.h/.ui` — Main window and UI
- `widgets/` — All UI components (login, reset, dashboard, etc.)
- `model/` — Data models
- `controller/` — Business logic
- `utils/` — Utilities (mail, bcrypt, etc.)
- `data/` — SQL schema and seed data

## Getting Started

1. **Clone the repository**
2. **Configure your MySQL database** (see `data/WaterSN_schema.sql`)
3. **Configure SMTP settings** in the code for password reset
4. **Build with CMake and Qt 6.9.1**
5. **Run the application**

## License

MIT License

---
<p align="center"><i>Made with ❤️ by gayensis for water management professionals</i></p>
