import os 
import pygeohash as pgh
import pandas as pd

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_PATH = os.path.join(BASE_DIR, 'dataset', 'train.csv')

def load_data(data_file_path : str):
  df = pd.read_csv(data_file_path)
  return df

def remove_na(df : pd.DataFrame) -> pd.DataFrame:
  df_cleaned = df.dropna()
  return df_cleaned

def onehotencode_columns(df_cleaned: pd.DataFrame) -> pd.DataFrame:

  df_encoded = pd.get_dummies(df_cleaned, columns=['Weather'], prefix='Weather')
  df_encoded.dropna(subset=['Temperature'], inplace=True)

  df_encoded = pd.get_dummies(df_encoded, columns=['RoadType'], prefix='Road')
  df_encoded['LargeVehicles'] = df_encoded['LargeVehicles'].map({'Allowed' : 1, 'Not Allowed' : 0})
  df_encoded['Landmarks'] = df_encoded['Landmarks'].map({'Yes' : 1, 'No' : 0})

  df_encoded['latitude'] = None
  df_encoded['longitude'] = None
  
  return df_encoded


#decode the geohash string against each record and save in latitude and longitude columns
def decode_geohash(geohash_string):
  if pd.isna(geohash_string) or geohash_string == '':
    return None, None
  else:
    return pgh.decode(geohash_string)
def save_lat_lon(df_encoded : pd.DataFrame) -> pd.DataFrame:
  df_encoded[['latitude', 'longitude']] = df_encoded['geohash'].apply(lambda x : pd.Series(decode_geohash(x)))
  #remove the unnecessary geohash string column now
  df_encoded = df_encoded.drop(columns = ['geohash'])
  return df_encoded


def main():
  df = load_data(DATA_PATH)
  df_cleaned = remove_na(df)
  df_encoded = onehotencode_columns(df_cleaned)
  df_encoded = save_lat_lon(df_encoded)

  print(df_encoded.count())

if __name__ == "__main__":
  main()


