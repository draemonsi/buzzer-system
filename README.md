# ⚙️ Software Setup Guide – Buzzer System

Welcome to the Software Team.

Before contributing, you must complete this setup to ensure everyone is working in the same environment.

---

## 🧠 1. Install Required Tools

### 1.1 Install Visual Studio Code
Download and install:
https://code.visualstudio.com/

During installation:
- Enable "Add to PATH"

---

### 1.2 Install Git

Download:
https://git-scm.com/

Verify installation:

```git --version```

---

## 🔧 2. Install VS Code Extensions

Open terminal and run:
```
code --install-extension platformio.platformio-ide  
code --install-extension GitHub.vscode-pull-request-github  
code --install-extension esbenp.prettier-vscode  
code --install-extension PKief.material-icon-theme  
code --install-extension ms-vscode.cpptools  
code --install-extension usernamehw.errorlens  
```
---

## 🔌 3. PlatformIO Setup

1. Open VS Code  
2. Click the PlatformIO icon (alien icon 👽)  
3. Click "New Project"

Set:
- Board: Arduino Mega 2560  
- Framework: Arduino  

---

## 🧠 4. Clone the Repository

Run in terminal:
```
git clone https://github.com/draemonsi/buzzer-system.git
cd buzzer-system  
code .  
```
---

## 🌿 5. Branching Workflow (IMPORTANT)

🚨 DO NOT push directly to main 🚨

### Create your own branch:
```
git checkout -b feature-yourname-task  
```
Example:
```
git checkout -b feature-andrei-input-system  
```
---

### After making changes:
```
git add .  
git commit -m "your message"  
git push origin feature-yourname-task  
```
Then create a Pull Request on GitHub.

---

## 🧩 6. Project Structure

code/  
  main/  
  modules/  
simulation/  
docs/  

- main/ → main system code  
- modules/ → input, logic, output  
- simulation/ → test files  
- docs/ → diagrams  

---

## 🧠 7. Team Rules

- Do NOT edit main directly  
- Do NOT overwrite others' code  
- Always use branches  
- Keep commits clear and meaningful  
- Test your code before pushing  

---

## 🏁 8. First Task

Once setup is complete:
- Confirm in the group chat  
- Wait for task assignment  

---

## ✅ 9. Setup Verification (REQUIRED)

Run the following command in your terminal:
```
echo "===== SETUP VERIFICATION =====" && \
echo "\n[VS CODE VERSION]" && code --version && \
echo "\n[GIT VERSION]" && git --version && \
echo "\n[PLATFORMIO EXTENSION]" && code --list-extensions | grep platformio && \
echo "\n[INSTALLED EXTENSIONS]" && code --list-extensions
```
---

### 📸 Screenshot Requirement

Take ONE screenshot showing the full output.

Make sure it includes:
- VS Code version  
- Git version  
- PlatformIO version  
- Installed extensions  

---

### 📤 Submission

Send your screenshot in the group chat.

---

### ⚠️ Note

If any part fails:
- Fix your setup first  
- Ask in the group chat  

### 🏁 Completion

No screenshot = not setup ❌  
