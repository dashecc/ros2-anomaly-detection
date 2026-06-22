import os

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
os.environ['TF_ENABLE_ONEDNN_OPTS'] = '0'

import tensorflow as tf
import numpy as np
from multiprocessing.connection import Listener

def run_engine():
    model_path = os.path.expanduser('./model_W20_U64_mae.keras')

    with tf.device('/CPU:0'):
        model = tf.keras.models.load_model(model_path)

    add = ('localhost', 6000)
    listener = Listener(add, authkey=b'setakey')


    while True:
        try:
            conn = listener.accept()
            while True:
                data = conn.recv()
                print(data)
                input_data = np.expand_dims(data, axis=0)

                prediction = model.predict(input_data, verbose=0)

                conn.send(prediction.tolist())
        except (EOFError, ConnectionRefusedError):
            print('Node disconnected.. waiting')
            continue
        except Exception as e:
            print(f'Engine error: {e}')
            break

if __name__ == "__main__":
    run_engine()