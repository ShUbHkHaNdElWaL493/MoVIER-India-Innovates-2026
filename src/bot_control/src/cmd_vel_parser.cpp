#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <std_msgs/msg/float64_multi_array.hpp>

class CmdVelParserNode : public rclcpp::Node
{

    private:

        rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_subscriber;
        rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr velocity_publisher;

        void cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
        {

            // Calculate translational and rotational velocity contributions
            const double linear_component = msg->linear.x;
            const double angular_component = msg->angular.z * this->get_parameter("track_width").as_double() / 2.0;

            // Calculate desired linear velocity for each wheel (m/s)
            const double velocity_left = linear_component - angular_component;
            const double velocity_right = linear_component + angular_component;

            // Convert linear velocity (m/s) to angular velocity (rad/s)
            const double omega_left = velocity_left / this->get_parameter("wheel_radius").as_double();;
            const double omega_right = velocity_right / this->get_parameter("wheel_radius").as_double();;

            auto command_msg = std::make_unique<std_msgs::msg::Float64MultiArray>();
            command_msg->data.push_back(omega_left);
            command_msg->data.push_back(omega_right);
            velocity_publisher->publish(std::move(command_msg));

        }
    
    public:
    
        CmdVelParserNode() : Node("teleop_parser")
        {

            this->declare_parameter<double>("wheel_radius", 0.05);
            this->declare_parameter<double>("track_width", 0.3);

            velocity_publisher = this->create_publisher<std_msgs::msg::Float64MultiArray>(
                "/forward_velocity_controller/commands",
                10
            );
            cmd_vel_subscriber = this->create_subscription<geometry_msgs::msg::Twist>(
                "/cmd_vel",
                10,
                std::bind(&CmdVelParserNode::cmd_vel_callback, this, std::placeholders::_1)
            );

        }

};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CmdVelParserNode>());
    rclcpp::shutdown();
    return 0;
}