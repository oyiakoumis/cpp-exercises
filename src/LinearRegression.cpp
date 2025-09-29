#include <iostream>
#include <vector>

class LinearRegression {
  private:
    std::vector<double> m_coefficients;
    double m_intercept;
    double m_learningRate;
    int m_maxIterations;

  public:
    LinearRegression(double learningRate = 0.01, int maxIterations = 1000)
        : m_intercept(0.0), m_learningRate(learningRate), m_maxIterations(maxIterations) {}

    void fit(const std::vector<std::vector<double>> &X, const std::vector<double> &y) {
        if (X.empty() || y.empty() || X.size() != y.size()) {
            return; // Handle edge case
        }

        int n_samples = X.size();
        int n_features = X[0].size();

        // Initialize coefficients to zero
        m_coefficients.assign(n_features, 0.0);
        m_intercept = 0.0;

        // Gradient descent
        for (int iter = 0; iter < m_maxIterations; ++iter) {
            // Calculate predictions
            std::vector<double> predictions(n_samples);
            for (int i = 0; i < n_samples; ++i) {
                predictions[i] = m_intercept;
                for (int j = 0; j < n_features; ++j) {
                    predictions[i] += m_coefficients[j] * X[i][j];
                }
            }

            // Calculate gradients
            std::vector<double> coeff_gradients(n_features, 0.0);
            double intercept_gradient = 0.0;

            for (int i = 0; i < n_samples; ++i) {
                double error = predictions[i] - y[i];
                intercept_gradient += error;
                for (int j = 0; j < n_features; ++j) {
                    coeff_gradients[j] += error * X[i][j];
                }
            }

            // Update parameters
            m_intercept -= m_learningRate * (2.0 / n_samples) * intercept_gradient;
            for (int j = 0; j < n_features; ++j) {
                m_coefficients[j] -= m_learningRate * (2.0 / n_samples) * coeff_gradients[j];
            }
        }
    }

    std::vector<double> predict(const std::vector<std::vector<double>> &X) const {
        std::vector<double> predictions;
        predictions.reserve(X.size());

        for (const auto &sample : X) {
            double prediction = m_intercept;
            for (size_t j = 0; j < sample.size() && j < m_coefficients.size(); ++j) {
                prediction += m_coefficients[j] * sample[j];
            }
            predictions.push_back(prediction);
        }

        return predictions;
    }

    std::vector<double> getCoefficients() const {
        return m_coefficients;
    }

    double getIntercept() const {
        return m_intercept;
    }

    double getMeanSquaredError(const std::vector<std::vector<double>> &X,
                               const std::vector<double> &y) const {
        if (X.size() != y.size() || X.empty()) {
            return 0.0;
        }

        auto predictions = predict(X);
        double mse = 0.0;

        for (size_t i = 0; i < y.size(); ++i) {
            double error = predictions[i] - y[i];
            mse += error * error;
        }

        return mse / y.size();
    }
};

// Test the implementation
int main() {
    // Data: price = a*size + b*bedrooms + c
    std::vector<std::vector<double>> X = {{1000, 2}, {1500, 3}, {2000, 4}, {2500, 5}, {1200, 2}};
    std::vector<double> y = {150000, 200000, 250000, 300000, 160000};

    LinearRegression lr(0.000001, 2000); // Small learning rate for large numbers
    lr.fit(X, y);

    auto predictions = lr.predict(X);
    auto coefficients = lr.getCoefficients();

    std::cout << "Trained model:" << std::endl;
    std::cout << "Intercept: " << lr.getIntercept() << std::endl;
    for (size_t i = 0; i < coefficients.size(); ++i) {
        std::cout << "Coefficient " << i << ": " << coefficients[i] << std::endl;
    }

    std::cout << "\nPredictions vs Actual:" << std::endl;
    for (size_t i = 0; i < y.size(); ++i) {
        std::cout << "Predicted: " << predictions[i] << ", Actual: " << y[i] << std::endl;
    }

    std::cout << "\nMSE: " << lr.getMeanSquaredError(X, y) << std::endl;

    // Test with new data
    std::vector<std::vector<double>> test_X = {{1800, 3}, {2200, 4}};
    auto test_predictions = lr.predict(test_X);

    std::cout << "\nNew predictions:" << std::endl;
    for (size_t i = 0; i < test_X.size(); ++i) {
        std::cout << "Size: " << test_X[i][0] << ", Bedrooms: " << test_X[i][1]
                  << " -> Price: " << test_predictions[i] << std::endl;
    }

    return 0;
}