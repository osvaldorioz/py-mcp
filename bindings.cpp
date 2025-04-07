#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <sw/redis++/redis++.h>
#include <Eigen/Dense>
#include <stdexcept>
#include <vector>
#include <random>
#include <cmath>  // Para std::sqrt

namespace py = pybind11;

class MCPClient {
private:
    sw::redis::Redis redis;

    std::vector<double> get_simulated_sensor_data() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> temp_dist(20.0, 40.0);
        std::uniform_real_distribution<> press_dist(950.0, 1050.0);
        return {temp_dist(gen), press_dist(gen)};
    }

public:
    MCPClient(const std::string& redis_uri = "redis://127.0.0.1:6379") : redis(redis_uri) {}

    Eigen::MatrixXd get_sensor_data() {
        try {
            auto data = get_simulated_sensor_data();
            Eigen::MatrixXd result(1, 2);
            result << data[0], data[1];
            redis.set("sensor_data", std::to_string(data[0]) + "," + std::to_string(data[1]));
            return result;
        } catch (const std::exception& e) {
            throw std::runtime_error("Error obteniendo datos del sensor: " + std::string(e.what()));
        }
    }

    Eigen::MatrixXd perform_clustering(const Eigen::MatrixXd& data) {
        try {
            if (data.rows() < 2) {
                throw std::runtime_error("Datos insuficientes para clustering");
            }
            Eigen::MatrixXd centroids(2, 2);
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> idx(0, data.rows() - 1);
            centroids.row(0) = data.row(idx(gen));
            centroids.row(1) = data.row(idx(gen));
            for (int iter = 0; iter < 10; ++iter) {
                std::vector<int> assignments(data.rows(), 0);
                for (int i = 0; i < data.rows(); ++i) {
                    double dist0 = (data.row(i) - centroids.row(0)).squaredNorm();
                    double dist1 = (data.row(i) - centroids.row(1)).squaredNorm();
                    assignments[i] = dist0 < dist1 ? 0 : 1;
                }
                Eigen::MatrixXd new_centroids = Eigen::MatrixXd::Zero(2, 2);
                std::vector<int> counts(2, 0);
                for (int i = 0; i < data.rows(); ++i) {
                    new_centroids.row(assignments[i]) += data.row(i);
                    counts[assignments[i]]++;
                }
                for (int k = 0; k < 2; ++k) {
                    if (counts[k] > 0) {
                        new_centroids.row(k) /= counts[k];
                    }
                }
                centroids = new_centroids;
            }
            return centroids;
        } catch (const std::exception& e) {
            throw std::runtime_error("Error en clustering: " + std::string(e.what()));
        }
    }

    Eigen::VectorXi detect_anomalies(const Eigen::MatrixXd& data, const Eigen::MatrixXd& centroids) {
        try {
            Eigen::VectorXd distances(data.rows());
            for (int i = 0; i < data.rows(); ++i) {
                double dist0 = (data.row(i) - centroids.row(0)).squaredNorm();
                double dist1 = (data.row(i) - centroids.row(1)).squaredNorm();
                distances(i) = std::min(dist0, dist1);
            }
            double mean = distances.mean();
            // Calcular desviación estándar manualmente
            double variance = 0.0;
            for (int i = 0; i < distances.size(); ++i) {
                variance += (distances(i) - mean) * (distances(i) - mean);
            }
            variance /= distances.size();
            double stddev = std::sqrt(variance);
            double threshold = mean + 2 * stddev;
            Eigen::VectorXi anomalies(data.rows());
            for (int i = 0; i < data.rows(); ++i) {
                anomalies(i) = distances(i) > threshold ? 1 : 0;
            }
            return anomalies;
        } catch (const std::exception& e) {
            throw std::runtime_error("Error detectando anomalías: " + std::string(e.what()));
        }
    }
};

PYBIND11_MODULE(mcp_module, m) {
    m.doc() = "Módulo MCP con Eigen y Redis";
    py::class_<MCPClient>(m, "MCPClient")
        .def(py::init<>())
        .def("get_sensor_data", &MCPClient::get_sensor_data)
        .def("perform_clustering", &MCPClient::perform_clustering)
        .def("detect_anomalies", &MCPClient::detect_anomalies);
}