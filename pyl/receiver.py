from patterns.ReqRespPattern import ReqRespPattern
import time

# Create and setup client
client = ReqRespPattern()
client.setup_client("test_server")

try:
    while True:
        # Send request and get response
        response = client.request("test_server", {"message": "Hello Server!"})
        print(f"Received response: {response}")
        time.sleep(1)
except KeyboardInterrupt:
    client.close()