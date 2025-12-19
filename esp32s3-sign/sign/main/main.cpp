#include <esp_log.h>
#include <clwe/keygen.hpp>
#include <clwe/sign.hpp>
#include <clwe/verify.hpp>

static const char *TAG = "ColorSign_Test";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting ColorSign basic operations test");

    try {
        // Create ColorSign instance with ML-DSA-44 parameters
        clwe::CLWEParameters params(44);
        clwe::ColorSignKeyGen keygen(params);
        clwe::ColorSign signer(params);
        clwe::ColorSignVerify verifier(params);

        ESP_LOGI(TAG, "ColorSign instance created with security level %d", params.security_level);

        // Generate key pair
        ESP_LOGI(TAG, "Generating key pair...");
        auto keypair = keygen.generate_keypair();
        auto pk = keypair.first;
        auto sk = keypair.second;
        ESP_LOGI(TAG, "Key pair generated successfully");

        // Prepare message
        std::string message_str = "Hello, ESP32-S3!";
        std::vector<uint8_t> message(message_str.begin(), message_str.end());

        // Sign message
        ESP_LOGI(TAG, "Signing message...");
        clwe::ColorSignature signature = signer.sign_message(message, sk, pk);
        ESP_LOGI(TAG, "Message signed successfully");

        // Verify signature
        ESP_LOGI(TAG, "Verifying signature...");
        bool is_valid = verifier.verify_signature(pk, signature, message);
        ESP_LOGI(TAG, "Signature verification completed");

        // Check result
        if (is_valid) {
            ESP_LOGI(TAG, "SUCCESS: Signature is valid - ColorSign operations working correctly");
        } else {
            ESP_LOGE(TAG, "FAILURE: Signature is invalid");
        }

        ESP_LOGI(TAG, "ColorSign test completed");

    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Exception occurred: %s", e.what());
    }
}