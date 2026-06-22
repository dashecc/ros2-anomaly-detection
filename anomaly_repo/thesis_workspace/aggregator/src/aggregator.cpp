#include <rclcpp/rclcpp.hpp>
#include <vector>
#include <map>
#include <fstream>
#include <mutex>
#include <chrono>
#include "std_msgs/msg/float32_multi_array.hpp"

#include "n2k_ros2_shared_msgs/msg/actual_pressure130314.hpp"
#include "n2k_ros2_shared_msgs/msg/ais_aids_to_navigation_report129041.hpp"
#include "n2k_ros2_shared_msgs/msg/ais_class_a_position_report129038.hpp"
#include "n2k_ros2_shared_msgs/msg/ais_class_a_static_and_voyage_related_data129794.hpp"
#include "n2k_ros2_shared_msgs/msg/ais_class_b_extended_position_report129040.hpp"
#include "n2k_ros2_shared_msgs/msg/ais_class_b_position_report129039.hpp"
#include "n2k_ros2_shared_msgs/msg/ais_class_b_static_data_part_a129809.hpp"
#include "n2k_ros2_shared_msgs/msg/ais_class_b_static_data_part_b129810.hpp"
#include "n2k_ros2_shared_msgs/msg/ais_safety_related_broadcast_message129802.hpp"
#include "n2k_ros2_shared_msgs/msg/ais_sar_aircraft_position_report129798.hpp"
#include "n2k_ros2_shared_msgs/msg/ais_utc_and_date_report129793.hpp"
#include "n2k_ros2_shared_msgs/msg/attitude127257.hpp"
#include "n2k_ros2_shared_msgs/msg/binary_switch_bank_status127501.hpp"
#include "n2k_ros2_shared_msgs/msg/cog_sog_rapid_update129026.hpp"
#include "n2k_ros2_shared_msgs/msg/direction_data130577.hpp"
#include "n2k_ros2_shared_msgs/msg/engine_parameters_rapid_update127488.hpp"
#include "n2k_ros2_shared_msgs/msg/environmental_parameters130310.hpp"
#include "n2k_ros2_shared_msgs/msg/environmental_parameters130311.hpp"
#include "n2k_ros2_shared_msgs/msg/furuno_heave65280.hpp"
#include "n2k_ros2_shared_msgs/msg/furuno_heel_angle_roll_information130843.hpp"
#include "n2k_ros2_shared_msgs/msg/gnss_dops129539.hpp"
#include "n2k_ros2_shared_msgs/msg/gnss_position_data129029.hpp"
#include "n2k_ros2_shared_msgs/msg/gnss_position_rapid_update129025.hpp"
#include "n2k_ros2_shared_msgs/msg/gnss_satellites_in_view129540.hpp"
#include "n2k_ros2_shared_msgs/msg/heave127252.hpp"
#include "n2k_ros2_shared_msgs/msg/magnetic_variation127258.hpp"
#include "n2k_ros2_shared_msgs/msg/meteorological_station_data130323.hpp"
#include "n2k_ros2_shared_msgs/msg/nmea2k_common_msg_header_struct.hpp"
#include "n2k_ros2_shared_msgs/msg/nmea2k_gnss_satellites_data_struct.hpp"
#include "n2k_ros2_shared_msgs/msg/product_info126996.hpp"
#include "n2k_ros2_shared_msgs/msg/rate_of_turn127251.hpp"
#include "n2k_ros2_shared_msgs/msg/reference_stations_struct.hpp"
#include "n2k_ros2_shared_msgs/msg/rosidl_generator_cpp__visibility_control.hpp"
#include "n2k_ros2_shared_msgs/msg/rudder127245.hpp"
#include "n2k_ros2_shared_msgs/msg/speed128259.hpp"
#include "n2k_ros2_shared_msgs/msg/system_time126992.hpp"
#include "n2k_ros2_shared_msgs/msg/temperature130312.hpp"
#include "n2k_ros2_shared_msgs/msg/temperature_extended_range130316.hpp"
#include "n2k_ros2_shared_msgs/msg/transmission_parameters_dynamic127493.hpp"
#include "n2k_ros2_shared_msgs/msg/vessel_heading127250.hpp"
#include "n2k_ros2_shared_msgs/msg/vessel_speed_components130578.hpp"
#include "n2k_ros2_shared_msgs/msg/wind_data130306.hpp"


struct PGNMetrics {
    int count = 0;
    int iat_count = 0;
    int jitter_count = 0;
    double sum_iat = 0.0;
    double sum_jitter = 0.0;
    double max_iat = 0.0;
    };

class Aggregator : public rclcpp::Node {
public:
    Aggregator() : Node("aggregator") {
        csv_file_.open("training_data.csv");
        csv_file_ << "bin_id,pgn,count,iat_count,jitter_count,"
             "sum_iat,max_iat,sum_jitter\n";
    
        publisher_ = this->create_publisher<std_msgs::msg::Float32MultiArray>("vessel_telemetry_windows", 10);

        create_sub<n2k_ros2_shared_msgs::msg::ActualPressure130314>("/t130314");
        create_sub<n2k_ros2_shared_msgs::msg::AisAidsToNavigationReport129041>("/t129041");
        create_sub<n2k_ros2_shared_msgs::msg::AisClassAPositionReport129038>("/t129038");
        create_sub<n2k_ros2_shared_msgs::msg::AisClassAStaticAndVoyageRelatedData129794>("/t129794");
        create_sub<n2k_ros2_shared_msgs::msg::AisClassBExtendedPositionReport129040>("/t129040");
        create_sub<n2k_ros2_shared_msgs::msg::AisClassBPositionReport129039>("/t129039");
        create_sub<n2k_ros2_shared_msgs::msg::AisClassBStaticDataPartA129809>("/t129809");
        create_sub<n2k_ros2_shared_msgs::msg::AisClassBStaticDataPartB129810>("/t129810");
        create_sub<n2k_ros2_shared_msgs::msg::AisSafetyRelatedBroadcastMessage129802>("/t129802");
        create_sub<n2k_ros2_shared_msgs::msg::AisSarAircraftPositionReport129798>("/t129798");
        create_sub<n2k_ros2_shared_msgs::msg::AisUtcAndDateReport129793>("/t129793");
        create_sub<n2k_ros2_shared_msgs::msg::Attitude127257>("/t127257");
        create_sub<n2k_ros2_shared_msgs::msg::BinarySwitchBankStatus127501>("/t127501");
        create_sub<n2k_ros2_shared_msgs::msg::CogSogRapidUpdate129026>("/t129026");
        create_sub<n2k_ros2_shared_msgs::msg::DirectionData130577>("/t130577");
        create_sub<n2k_ros2_shared_msgs::msg::EngineParametersRapidUpdate127488>("/t127488");
        create_sub<n2k_ros2_shared_msgs::msg::EnvironmentalParameters130310>("/t130310");
        create_sub<n2k_ros2_shared_msgs::msg::EnvironmentalParameters130311>("/t130311");
        create_sub<n2k_ros2_shared_msgs::msg::FurunoHeave65280>("/t65280");
        create_sub<n2k_ros2_shared_msgs::msg::FurunoHeelAngleRollInformation130843>("/t130843");
        create_sub<n2k_ros2_shared_msgs::msg::GnssDops129539>("/t129539");
        create_sub<n2k_ros2_shared_msgs::msg::GnssPositionData129029>("/t129029");
        create_sub<n2k_ros2_shared_msgs::msg::GnssPositionRapidUpdate129025>("/t129025");
        create_sub<n2k_ros2_shared_msgs::msg::GnssSatellitesInView129540>("/t129540");
        create_sub<n2k_ros2_shared_msgs::msg::Heave127252>("/t127252");
        create_sub<n2k_ros2_shared_msgs::msg::MagneticVariation127258>("/t127258");
        create_sub<n2k_ros2_shared_msgs::msg::MeteorologicalStationData130323>("/t130323");
        create_sub<n2k_ros2_shared_msgs::msg::ProductInfo126996>("/t126996");
        create_sub<n2k_ros2_shared_msgs::msg::RateOfTurn127251>("/t127251");
        create_sub<n2k_ros2_shared_msgs::msg::Rudder127245>("/t127245");
        create_sub<n2k_ros2_shared_msgs::msg::Speed128259>("/t128259");
        create_sub<n2k_ros2_shared_msgs::msg::SystemTime126992>("/t126992");
        create_sub<n2k_ros2_shared_msgs::msg::Temperature130312>("/t130312");
        create_sub<n2k_ros2_shared_msgs::msg::TemperatureExtendedRange130316>("/t130316");
        create_sub<n2k_ros2_shared_msgs::msg::TransmissionParametersDynamic127493>("/t127493");
        create_sub<n2k_ros2_shared_msgs::msg::VesselHeading127250>("/t127250");
        create_sub<n2k_ros2_shared_msgs::msg::VesselSpeedComponents130578>("/t130578");
        create_sub<n2k_ros2_shared_msgs::msg::WindData130306>("/t130306");

        timer_ = this->create_wall_timer(std::chrono::milliseconds(100),
            std::bind(&Aggregator::publish_telemetry, this));
    }

private:
    std::vector<rclcpp::SubscriptionBase::SharedPtr> subs_;
    rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr publisher_;
    std::map<uint64_t, std::map<uint32_t, PGNMetrics>> pgn_data_;
    std::map <uint32_t, double> last_arrival_map_;
    std::map <uint32_t, double> last_iat_map_;
    std::ofstream csv_file_;

    std::mutex mtx_;
    rclcpp::TimerBase::SharedPtr timer_;
    double last_latency = 0.0;

    template<typename T>
    void create_sub(const std::string& topic) {
        auto sub = this->create_subscription<T>(
            topic,
            10,
            std::bind(&Aggregator::callback<T>, this, std::placeholders::_1)
        );
        subs_.push_back(sub);
    }

    template<typename T>
    void callback(const typename T::SharedPtr msg) {
        std::lock_guard<std::mutex> lock(mtx_);

        int64_t arrival_time = msg->header.epoch_timestamp;
        uint32_t pgn_val = msg->header.pgn;
        uint64_t bin_id = arrival_time / 1000;
        auto& m = pgn_data_[bin_id][pgn_val];

        m.count++;

        double iat = 0.0;
        if (last_arrival_map_.count(pgn_val)) {

            iat = arrival_time - last_arrival_map_[pgn_val];
            if (last_iat_map_.count(pgn_val)) {
                double jitter = std::abs(iat - last_iat_map_[pgn_val]);
                m.sum_jitter += jitter;
                m.jitter_count++;
            }

            m.sum_iat += iat;
            m.iat_count++;
            last_iat_map_[pgn_val] = iat;

            m.max_iat = std::max(m.max_iat, iat);
        }
        last_arrival_map_[pgn_val] = arrival_time;

    }

    void publish_telemetry() {
        std::map<uint64_t, std::map<uint32_t, PGNMetrics>> windows_to_send;
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (pgn_data_.empty()) return;

            uint64_t current_active_window = pgn_data_.rbegin()->first;

            auto it = pgn_data_.begin();
            while (it != pgn_data_.end() && it->first < current_active_window) {
                windows_to_send[it->first] = std::move(it->second);
                it = pgn_data_.erase(it);
            }
        }

        if (windows_to_send.empty()) return;
        auto message = std_msgs::msg::Float32MultiArray();

        for (auto const& [bin, pgns] : windows_to_send) {
            float safe_bin = static_cast<float>(bin % 100000);
            for (auto const& [pgn, m] : pgns) {
                message.data.push_back(static_cast<float>(safe_bin));
                message.data.push_back(static_cast<float>(pgn));
                message.data.push_back(static_cast<float>(m.count));
                message.data.push_back(static_cast<float>(m.iat_count));
                message.data.push_back(static_cast<float>(m.jitter_count));
                message.data.push_back(static_cast<float>(m.sum_iat));
                message.data.push_back(static_cast<float>(m.max_iat));
                message.data.push_back(static_cast<float>(m.sum_jitter));
                
                csv_file_ << safe_bin << "," << pgn << "," << m.count << "," << m.iat_count << ',' << m.jitter_count << "," << m.sum_iat << "," << m.max_iat << "," << m.sum_jitter << "\n";

            }
        }
        publisher_->publish(message);
    }
};

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Aggregator>());
    return 0;
}