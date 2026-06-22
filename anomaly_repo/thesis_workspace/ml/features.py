import pandas as pd
import json

df = pd.read_csv("training_data.csv")

unique_pgns = sorted(df['pgn'].unique())

features = []

for pgn in unique_pgns:
    features.append(f'count_{pgn}')
    features.append(f'iat_{pgn}')
    features.append(f'jitter_{pgn}')

with open('feature_table.json', 'w') as f:
    json.dump(features, f)