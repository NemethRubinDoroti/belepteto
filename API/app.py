from flask import Flask, request, jsonify
import sqlite3
from datetime import datetime

app = Flask(__name__)

def get_db_connection():
    conn = sqlite3.connect('db.db')
    conn.row_factory = sqlite3.Row
    return conn

@app.route('/felhasznalok', methods=['GET', 'POST'])
def felhasznalok():
    conn = get_db_connection()
    if request.method == 'GET':
        cursor = conn.execute('SELECT * FROM felhasznalok')
        users = cursor.fetchall()
        return jsonify([dict(user) for user in users]), 200
    elif request.method == 'POST':
        new_user = request.json
        conn.execute('INSERT INTO felhasznalok (nev, kreditsz, jog) VALUES (?, ?, ?)',
                     (new_user['nev'], new_user.get('kreditsz', 0), new_user['jog']))
        conn.commit()
        return "User added successfully", 201

@app.route('/felhasznalok/<string:card>', methods=['GET'])
def felhasznalo(card):
    conn = get_db_connection()
    cursor = conn.cursor()
    user = cursor.execute('SELECT * FROM felhasznalok WHERE card = ?', (card,)).fetchone()
    if user is None:
        return "User not found", 404
    current_time = datetime.now().time()
    if user['jog'] == 0 and (current_time < datetime.strptime('08:00', '%H:%M').time() or current_time > datetime.strptime('16:00', '%H:%M').time()):
        return "Forbidden", 403
    
    if user['jog'] == 0 and user['kreditsz'] == 0:
        return "Not enough credits", 402

    if user['jog'] == 0:
        conn.execute('UPDATE felhasznalok SET kreditsz = kreditsz - 1 WHERE card = ?', (card,))
    
    conn.commit()
    return jsonify(dict(user)), 200
    

@app.route('/log/<string:card>', methods=['POST'])
def log_entry(card):
    conn = get_db_connection()
    new_log = {
        'card': str(request.json.get("card")),
        'type': int(request.json.get('type')),
        'time': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    }
    conn.execute('INSERT INTO log (card, type, time) VALUES (?, ?, ?)',
                 (new_log['card'], new_log['type'], new_log['time']))
    conn.commit()
    return "Log entry added successfully", 201

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=10000)