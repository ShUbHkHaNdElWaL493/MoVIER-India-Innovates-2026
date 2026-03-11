#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu
from tf2_ros import TransformException
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener
import math

class YawComparator(Node):
    def __init__(self):
        super().__init__('yaw_comparator')
        
        # 1. Setup IMU Subscription
        self.imu_topic = '/imu/data_raw'
        self.subscription = self.create_subscription(
            Imu,
            self.imu_topic,
            self.imu_callback,
            10)
        self.latest_imu_yaw = 0.0
        self.got_imu = False

        # 2. Setup TF Listener (odom -> base_link)
        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)

        # 3. Setup a timer to compare them at 10Hz
        self.timer = self.create_timer(0.1, self.compare_yaw)
        
        self.get_logger().info("Starting Yaw Comparison. Waiting for TF and IMU...")

    def imu_callback(self, msg):
        q = msg.orientation
        _, _, yaw = self.euler_from_quaternion(q.x, q.y, q.z, q.w)
        self.latest_imu_yaw = yaw
        self.got_imu = True

    def compare_yaw(self):
        if not self.got_imu:
            return

        try:
            # Look up the latest transform from odom to base_link
            t = self.tf_buffer.lookup_transform(
                'odom',
                'base_link',
                rclpy.time.Time())
            
            # Extract quaternion from the transform
            q = t.transform.rotation
            _, _, tf_yaw = self.euler_from_quaternion(q.x, q.y, q.z, q.w)

            # Convert to degrees for easier reading
            imu_deg = math.degrees(self.latest_imu_yaw)
            tf_deg = math.degrees(tf_yaw)
            diff = imu_deg - tf_deg

            # Print the formatted comparison
            self.get_logger().info(
                f"IMU: {imu_deg:>7.2f}° | Odom TF: {tf_deg:>7.2f}° | Diff: {diff:>7.2f}°"
            )

        except TransformException as ex:
            self.get_logger().debug(f"Could not transform odom to base_link: {ex}")

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

        return roll_x, pitch_y, yaw_z # radians

def main(args=None):
    rclpy.init(args=args)
    node = YawComparator()
    
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
        
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
