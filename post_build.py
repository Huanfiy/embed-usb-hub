#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Post-build script for firmware information display
"""

import os
import sys
import subprocess
import re

# ANSI color codes
class Colors:
    RED = '\033[91m'
    YELLOW = '\033[93m'
    GREEN = '\033[92m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    WHITE = '\033[97m'
    BOLD = '\033[1m'
    END = '\033[0m'

def run_command(cmd):
    """Execute command and return output"""
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        return result.stdout.strip(), result.stderr.strip(), result.returncode
    except Exception as e:
        return "", str(e), 1

def get_file_size(filepath):
    """Get file size in bytes"""
    try:
        return os.path.getsize(filepath)
    except:
        return 0

def format_size(size_bytes):
    """Format file size for display"""
    if size_bytes < 1024:
        return f"{size_bytes:>4}B"
    elif size_bytes < 1024 * 1024:
        return f"{size_bytes/1024:>5.1f}K"
    else:
        return f"{size_bytes/(1024*1024):>5.2f}M"

def parse_size_output(size_output):
    """Parse arm-none-eabi-size output"""
    lines = size_output.strip().split('\n')
    if len(lines) < 2:
        return None
    
    for line in lines[1:]:
        parts = line.split()
        if len(parts) >= 4 and parts[0].isdigit():
            return {
                'text': int(parts[0]),
                'data': int(parts[1]),
                'bss': int(parts[2]),
                'total': int(parts[3])
            }
    return None

def detect_build_mode(elf_file):
    """Detect if firmware is built in debug or release mode"""
    # Check for debug symbols
    objdump_cmd = f"arm-none-eabi-objdump -h {elf_file}"
    output, error, ret = run_command(objdump_cmd)
    
    if ret != 0:
        return "Unknown"
    
    # Look for debug sections
    debug_sections = ['.debug_info', '.debug_line', '.debug_frame', '.debug_str']
    has_debug = any(section in output for section in debug_sections)
    
    if has_debug:
        return "Debug"
    else:
        return "Release"

def create_progress_bar(percentage, width=30):
    """Create download-style progress bar"""
    filled = int(width * percentage / 100)
    bar = '=' * filled
    if filled < width:
        bar += '>'
        bar += ' ' * (width - filled - 1)
    else:
        bar = '=' * width
    return f"[{bar}]"

def print_firmware_info(elf_file, bin_file):
    """Print firmware information"""
    
    # STM32F446RC specifications
    FLASH_SIZE = 256 * 1024  # 256KB
    RAM_SIZE = 128 * 1024    # 128KB
    
    # Get size information
    size_cmd = f"arm-none-eabi-size --format=berkeley {elf_file}"
    size_output, size_error, size_ret = run_command(size_cmd)
    
    if size_ret != 0:
        print(f"{Colors.RED}Failed to get firmware info: {size_error}{Colors.END}")
        return
    
    size_info = parse_size_output(size_output)
    if not size_info:
        print(f"{Colors.RED}Failed to parse firmware info{Colors.END}")
        return
    
    # Calculate usage
    flash_used = size_info['text'] + size_info['data']
    ram_used = size_info['data'] + size_info['bss']
    
    flash_pct = (flash_used / FLASH_SIZE) * 100
    ram_pct = (ram_used / RAM_SIZE) * 100
    
    bin_size = get_file_size(bin_file)
    elf_size = get_file_size(elf_file)
    
    # Detect build mode
    build_mode = detect_build_mode(elf_file)
    
    # Output firmware information
    print("=" * 70)
    mode_color = Colors.BLUE if build_mode == "Debug" else Colors.GREEN
    print(f"{Colors.BOLD}{Colors.CYAN}STM32F446RC Firmware Information{Colors.END} -- {mode_color}{build_mode}{Colors.END}")
    print("=" * 70)
    
    # Memory usage
    print(f"{Colors.BOLD}Memory Usage:{Colors.END}")
    print(f"Flash: {format_size(flash_used)} / {format_size(FLASH_SIZE)} ({flash_pct:5.1f}%) {create_progress_bar(flash_pct)}")
    print(f"RAM  : {format_size(ram_used)} / {format_size(RAM_SIZE)} ({ram_pct:5.1f}%) {create_progress_bar(ram_pct)}")
    
    # Section details
    print()
    print(f"{Colors.BOLD}Section Details:{Colors.END}")
    print(f"ELF:{format_size(elf_size)}  BIN:{format_size(bin_size)} TEXT:{format_size(size_info['text'])}  DATA:{format_size(size_info['data'])}  BSS:{format_size(size_info['bss'])}")
    
    # File information
    # print()
    # print(f"{Colors.BOLD}Files:{Colors.END}")
    # print(f"ELF: {format_size(elf_size)}  BIN: {format_size(bin_size)}")
    
    # Status indication with colors
    print()
    if flash_pct > 90 or ram_pct > 90:
        print(f"{Colors.RED}{Colors.BOLD}WARNING: Memory usage is critically high!{Colors.END}")
    elif flash_pct > 80 or ram_pct > 80:
        print(f"{Colors.YELLOW}{Colors.BOLD}NOTICE: Memory usage is high{Colors.END}")
    else:
        print(f"{Colors.GREEN}{Colors.BOLD}Memory usage is normal{Colors.END}")
    
    print("=" * 70)

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 post_build.py <elf_file>")
        sys.exit(1)
    
    elf_file = sys.argv[1]
    bin_file = elf_file.replace('.elf', '.bin')
    
    # Ensure correct bin file path
    if not os.path.exists(bin_file):
        bin_file = os.path.join('build', os.path.basename(bin_file))
    
    if not os.path.exists(elf_file):
        print(f"{Colors.RED}ELF file not found: {elf_file}{Colors.END}")
        sys.exit(1)
    
    if not os.path.exists(bin_file):
        print(f"{Colors.RED}BIN file not found: {bin_file}{Colors.END}")
        sys.exit(1)
    
    print_firmware_info(elf_file, bin_file)

if __name__ == "__main__":
    main() 