from flask import Flask, render_template
import requests

app = Flask(__name__)

@app.route('/')
def home():
    url = "http://192.168.1.212/"
    response = requests.get(url)
    data = response.json()
    temp = data.get("temp")
    humidity = data.get("humidity")
    batt = data.get("batt")
    rssi = data.get("rssi")
    return render_template('index.html', temp=temp, humidity=humidity, batt=batt, rssi=rssi)

if __name__ == '__main__':
    app.run("0.0.0.0", 80)
