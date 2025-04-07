from mcp.server.fastmcp import FastMCP
import numpy as np
import time

# Crear el servidor MCP
mcp = FastMCP("SensorDataServer", host="localhost", port=8080)

# Definir el recurso con una URL completa
@mcp.resource("http://localhost:8080/sensor_data")
def get_sensor_data():
    """Devuelve datos de sensores simulados como CSV"""
    temp = np.random.normal(25, 5)  # Temperatura ~ N(25, 5)
    press = np.random.normal(1000, 50)  # Presión ~ N(1000, 50)
    timestamp = time.time()
    return f"{timestamp},{temp},{press}"

# Iniciar el servidor
if __name__ == "__main__":
    mcp.run()