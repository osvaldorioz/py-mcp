import matplotlib
matplotlib.use('TkAgg')  # Backend interactivo
import mcp_module
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

try:
    client = mcp_module.MCPClient()
except Exception as e:
    print(f"Error al inicializar MCPClient: {e}")
    exit(1)

fig, ax = plt.subplots()
sc = ax.scatter([], [], c='blue', label='Datos')
centroids_sc = ax.scatter([], [], c='red', marker='x', label='Centroides')
anomalies_sc = ax.scatter([], [], c='green', marker='o', label='Anomalías')
plt.legend()
ax.set_xlim(0, 50)
ax.set_ylim(900, 1100)
ax.set_xlabel('Temperatura (°C)')
ax.set_ylabel('Presión (hPa)')
plt.title('Clústeres de Contexto y Anomalías en Tiempo Real')

data_history = []

def update(frame):
    global data_history
    try:
        new_data = client.get_sensor_data()
        data_history.append(new_data[0])
        if len(data_history) > 50:
            data_history.pop(0)
        
        data_matrix = np.array(data_history)
        centroids = client.perform_clustering(data_matrix)
        anomalies = client.detect_anomalies(data_matrix, centroids)
        
        sc.set_offsets(data_matrix)
        centroids_sc.set_offsets(centroids)
        anomalies_data = data_matrix[anomalies.astype(bool)]
        anomalies_sc.set_offsets(anomalies_data if anomalies_data.size > 0 else np.empty((0, 2)))
        
        return sc, centroids_sc, anomalies_sc
    except Exception as e:
        print(f"Error en la actualización: {e}")
        return sc, centroids_sc, anomalies_sc

ani = FuncAnimation(fig, update, interval=1000, cache_frame_data=False)
plt.show()