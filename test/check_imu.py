#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu
import tkinter as tk
import math

class ImuSubscriber(Node):
    def __init__(self):
        super().__init__('imu_compass_gui')
        
        # Change '/imu/data' to match your actual IMU topic name
        self.imu_topic = '/imu/data_raw' 
        
        self.subscription = self.create_subscription(
            Imu,
            self.imu_topic,
            self.listener_callback,
            10)
            
        self.roll = 0.0
        self.pitch = 0.0
        self.yaw = 0.0
        
        self.get_logger().info(f"Listening to IMU data on {self.imu_topic}...")

    def listener_callback(self, msg):
        q = msg.orientation
        self.roll, self.pitch, self.yaw = self.euler_from_quaternion(q.x, q.y, q.z, q.w)

    def euler_from_quaternion(self, x, y, z, w):
        """Convert a quaternion into euler angles (roll, pitch, yaw)"""
        t0 = +2.0 * (w * x + y * z)
        t1 = +1.0 - 2.0 * (x * x + y * y)
        roll_x = math.atan2(t0, t1)

        t2 = +2.0 * (w * y - z * x)
        t2 = +1.0 if t2 > +1.0 else t2
        t2 = -1.0 if t2 < -1.0 else t2
        pitch_y = math.asin(t2)

        t3 = +2.0 * (w * z + x * y)
        t4 = +1.0 - 2.0 * (y * y + z * z)
        yaw_z = math.atan2(t3, t4)

        return roll_x, pitch_y, yaw_z # returned in radians

class CompassGUI:
    def __init__(self, root, node):
        self.root = root
        self.node = node
        self.root.title("IMU Orientation Dashboard")
        self.root.configure(bg="#2d2d2d")

        self.size = 250
        self.center = self.size // 2
        self.radius = self.size // 2 - 30

        self.canvases = {}
        self.labels = {}
        self.needles = {}

        angles = ["Roll", "Pitch", "Yaw"]
        
        # Build the 3 compasses side-by-side
        for i, angle in enumerate(angles):
            frame = tk.Frame(root, bg="#2d2d2d")
            frame.grid(row=0, column=i, padx=15, pady=20)

            # Title Label
            lbl = tk.Label(frame, text=angle, font=("Helvetica", 16, "bold"), fg="white", bg="#2d2d2d")
            lbl.pack()

            # Value Label
            val_lbl = tk.Label(frame, text="0.0°", font=("Helvetica", 14), fg="#4CAF50", bg="#2d2d2d")
            val_lbl.pack(pady=5)
            self.labels[angle] = val_lbl

            # Canvas for drawing the compass
            canvas = tk.Canvas(frame, width=self.size, height=self.size, bg="#2d2d2d", highlightthickness=0)
            canvas.pack()

            # Draw compass outer ring and center dot
            canvas.create_oval(30, 30, self.size-30, self.size-30, outline="gray", width=4)
            canvas.create_oval(self.center-5, self.center-5, self.center+5, self.center+5, fill="white")
            
            # Draw needle (initially pointing straight up)
            needle = canvas.create_line(self.center, self.center, self.center, 30, fill="#ff5252", width=4, arrow=tk.LAST)
            
            self.canvases[angle] = canvas
            self.needles[angle] = needle

        # Start the GUI update loop
        self.update_gui()

    def update_gui(self):
        # Spin ROS node briefly to grab the latest IMU message
        rclpy.spin_once(self.node, timeout_sec=0.01)

        # Update the visuals based on the latest data
        self.update_needle("Roll", self.node.roll)
        self.update_needle("Pitch", self.node.pitch)
        self.update_needle("Yaw", self.node.yaw)

        # Schedule the next GUI update in 50ms (runs at ~20Hz)
        self.root.after(50, self.update_gui)

    def update_needle(self, name, angle_rad):
        # Convert to degrees for text display
        angle_deg = math.degrees(angle_rad)
        self.labels[name].config(text=f"{angle_deg:.1f}°")

        # Calculate new needle coordinates
        # 0 rad points UP. Positive rotation is clockwise.
        nx = self.center + self.radius * math.sin(angle_rad)
        ny = self.center - self.radius * math.cos(angle_rad)

        # Redraw needle
        self.canvases[name].coords(self.needles[name], self.center, self.center, nx, ny)

def main(args=None):
    rclpy.init(args=args)
    node = ImuSubscriber()
    
    root = tk.Tk()
    gui = CompassGUI(root, node)
    
    # Ensure ROS cleanly shuts down if the window is closed
    def on_closing():
        node.destroy_node()
        rclpy.shutdown()
        root.destroy()
        
    root.protocol("WM_DELETE_WINDOW", on_closing)
    
    # Start Tkinter main loop
    root.mainloop()

if __name__ == '__main__':
    main()
