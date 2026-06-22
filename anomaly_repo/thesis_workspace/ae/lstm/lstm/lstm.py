import rclpy
from rclpy.node import Node
from collections import deque
import numpy as np
import pandas as pd
import joblib
import json
import time
from multiprocessing.connection import Client

from std_msgs.msg import Float32MultiArray, String, UInt16


class Ros2LSTM(Node):
    def __init__(self):
        super().__init__('ros2lstm')

        self.sub = self.create_subscription(Float32MultiArray, '/vessel_telemetry_windows', self.listener_callback, 10)
        self.pub = self.create_publisher(String, '/anomaly_score', 10)
        self.binpub = self.create_publisher(UInt16, '/binid', 10)

        with open('../../ml/feature_table.json', 'r') as f:
            self.training_columns = json.load(f)
        
        
        self.scaler = joblib.load('../../ml/min_max_scaler.pkl')
        timer_period = 1 #seconds

        while True:
            try:
                #match with engine/mlserver.py, interal project.
                self.conn = Client(('localhost', 6000), authkey=b'setakey')
                break
            except ConnectionRefusedError:
                time.sleep(1)

        #Params
        self.latest_score = 0
        self.window_size = 20
        self.threshold = 0.085

        #Result tracking
        self.bin_num = 1

        self.buffer = deque(maxlen=self.window_size)

        self.timer = self.create_timer(timer_period, self.timer_callback)

        self.get_logger().info(f'Node started. Listening with window size {self.window_size}')


    def listener_callback(self, msg):

        matrix = np.array(msg.data).reshape(-1, 8)

        raw_df = pd.DataFrame(matrix, columns=[
            'bin_id',
            'pgn',
            'count',
            'iat_count',
            'jitter_count',
            'sum_iat',
            'max_iat',
            'sum_jitter',
            ])

        df_consolidate = raw_df.groupby(['bin_id', 'pgn']).agg({
            'count':'sum',
            'iat_count':'sum',
            'jitter_count':'sum',
            'sum_iat': 'sum',
            'max_iat': 'max',
            'sum_jitter':'sum',
        }).reset_index()

        df_consolidate['avg_iat'] = df_consolidate['sum_iat'] / df_consolidate['iat_count']
        df_consolidate['avg_jitter'] = df_consolidate['sum_jitter'] / df_consolidate['jitter_count']

        df_consolidate = df_consolidate.drop(columns=['sum_iat','sum_jitter','iat_count','jitter_count'])
        df_consolidate.replace([np.inf, -np.inf], np.nan, inplace=True)
        df_consolidate.fillna(0,inplace=True)

        pivoted = df_consolidate.pivot(index='bin_id', columns='pgn', values=['count', 'avg_iat', 'max_iat', 'avg_jitter'])

        pivoted.fillna(0, inplace=True)

        pivoted.columns = [f'{col[0]}_{int(col[1])}' for col in pivoted.columns] #Flatten

        final = pivoted.reindex(columns=self.training_columns, fill_value=0)

        scaled_data = self.scaler.transform(final)
        scaled_data = np.clip(scaled_data, 0, 1)
        self.buffer.append(scaled_data[0])

        if len(self.buffer) == self.window_size:
            input_window = np.array(self.buffer)
            self.conn.send(input_window)
            reconstruction = np.array(self.conn.recv())
            self.latest_score = float(np.mean(np.abs(input_window - reconstruction)))

            with open('../../results/results.csv', 'a') as f:
                f.write(f'{self.bin_num},{self.latest_score}\n')
            self.bin_num += 1

    def timer_callback(self):
        binid = UInt16()
        msg = String()
        if self.latest_score > self.threshold:
            self.get_logger().info(f'ANOMALY | MSE: {self.latest_score:.6f}')
        else:
            self.get_logger().info(f'NORMAL')
        binid.data = self.bin_num
        msg.data = json.dumps({'score' : self.latest_score})
        self.binpub.publish(binid)
        self.pub.publish(msg)

def main(args=None):
    rclpy.init(args=args)
    node = Ros2LSTM()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
