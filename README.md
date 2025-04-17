# Ibery++ Compiler

Ibery++ is a modern programming language compiler that can generate web code (HTML, CSS, JS) and execute code directly using a virtual machine.

## Features

- Compile ibery++ code to web technologies
- Execute code directly using a built-in virtual machine
- Interactive terminal for running commands and code
- System-wide installation support

## Installation

### Quick Install

```bash
# Download the repository
git clone https://github.com/yourusername/iberypp.git
cd iberypp

# Make the installation script executable
chmod +x install.sh

# Run the installation script as root
sudo ./install.sh
```

### Manual Installation

1. Clone the repository:
```bash
git clone https://github.com/yourusername/iberypp.git
cd iberypp
```

2. Build the compiler:
```bash
make
```

3. Install system-wide:
```bash
sudo cp iberypp /usr/local/bin/
```

## Usage

### Compile to Web Code
```bash
iberypp compile input.ibpp output_dir
```

### Host Web Application
```bash
iberypp host input.ibpp
```

### Run Code Directly
```bash
iberypp run input.ibpp
```

### Interactive Terminal
```bash
iberypp terminal
```

## File Extensions

- Source files: `.ibpp`
- Compiled HTML: `.html`
- Compiled CSS: `.css`
- Compiled JavaScript: `.js`

## Examples

### Basic Program
```ibery
function greet(name) {
    print("Hello, " + name + "!");
}

greet("World");
```

### Web Application
```ibery
class Button {
    text label;
    function onClick() {
        print("Button clicked!");
    }
}

Button myButton = new Button();
myButton.label = "Click me";
```

## Uninstallation

To remove ibery++ from your system:

```bash
# Run the uninstallation script as root
sudo ./uninstall.sh
```

## License

This project is licensed under the MIT License - see the LICENSE file for details. # ibery_Labinger
# ibery_Labinger
