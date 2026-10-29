import webbrowser
import os
import sys

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    frontend_path = os.path.join(script_dir, "frontend", "index.html")

    if not os.path.exists(frontend_path):
        print(f"[Error] Frontend file not found at: {frontend_path}")
        sys.exit(1)

    print("==================================================================")
    print("      Launching SmartFlow Graphical Traffic Controller UI         ")
    print("==================================================================")
    print(f"Opening: {frontend_path}")
    print("Opening in your default web browser (Chrome / Edge / Firefox)...")
    
    webbrowser.open(f"file://{frontend_path}")
    print("\n[Success] UI launched! You can interact with the visual dashboard.")

if __name__ == "__main__":
    main()
