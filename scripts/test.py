BASE_RESULTS_FOLDER_PATH = "C:/Users/Jonathon/Desktop/Results/"
SERVER_APPLICATION_PATH = "C:/Users/Jonathon/Desktop/Distributed Gaming Service/bin/Release-windows-x86_64/Server/Server.exe"
CLIENT_APPLICATION_PATH = "C:/Users/Jonathon/Desktop/Distributed Gaming Service/bin/Release-windows-x86_64/Client/Client.exe"

SERVER_PORT_NUMBER = 8080
MAX_APPLICATION_LIFE_MS = (60 * 1000)
NUMBER_OF_CLIENTS = list(range(1, 32 + 1))


# Import subprocess as a method of executing external processes (applications)
import subprocess

def run_server_app(results_folder_path):
    global SERVER_APPLICATION_PATH, SERVER_PORT_NUMBER, MAX_APPLICATION_LIFE_MS
    subprocess.run([SERVER_APPLICATION_PATH, "{}".format(SERVER_PORT_NUMBER), "{}".format(MAX_APPLICATION_LIFE_MS), results_folder_path])

def run_client_app(results_folder_path, id):
    global CLIENT_APPLICATION_PATH, SERVER_PORT_NUMBER, MAX_APPLICATION_LIFE_MS
    subprocess.run([CLIENT_APPLICATION_PATH, "127.0.0.1", "{}".format(SERVER_PORT_NUMBER), "{}".format(MAX_APPLICATION_LIFE_MS), "{}".format(id), results_folder_path])
    


# Import threading to execute portions of code in their own threads
import threading
import os

for current_number_of_clients in NUMBER_OF_CLIENTS:
    results_folder_path = BASE_RESULTS_FOLDER_PATH + "{} Players".format(current_number_of_clients)
    os.makedirs(results_folder_path, exist_ok=True)

    # Define a list to store any active threads (so that the program can wait for each thread to finish at a later point) 
    active_threads = []

    # Create, start and store a new thread that runs the server application and waits for it to complete
    server_thread = threading.Thread(target=run_server_app, kwargs={"results_folder_path": results_folder_path}, name="Server Thread")
    server_thread.start()
    active_threads.append(server_thread)

    # Create, start and store a number of new threads which each run the client application and wait for it to complete 
    for i in range(current_number_of_clients):
        client_thread = threading.Thread(target=run_client_app, kwargs={"results_folder_path": results_folder_path, "id": i}, name="Client Thread {}".format(i + 1))
        client_thread.start()
        active_threads.append(client_thread)

    # Now wait for all threads (server and client applications) to complete
    for current_thread in active_threads:
        current_thread.join()