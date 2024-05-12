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

@app.route('/felhasznalok/<int:id>', methods=['GET', 'PUT', 'DELETE'])
def felhasznalo(id):
    conn = get_db_connection()
    cursor = conn.cursor()

    if request.method == 'GET':
        user = cursor.execute('SELECT * FROM felhasznalok WHERE id = ?', (id,)).fetchone()
        if user is None:
            return "User not found", 404
        return jsonify(dict(user)), 200
    elif request.method == 'PUT':
        updated_user = request.json
        cursor.execute('UPDATE felhasznalok SET nev = ?, kreditsz = ?, jog = ? WHERE id = ?',
                       (updated_user['nev'], updated_user.get('kreditsz', 0), updated_user['jog'], id))
        conn.commit()
        return "User updated successfully", 200
    elif request.method == 'DELETE':
        cursor.execute('DELETE FROM felhasznalok WHERE id = ?', (id,))
        conn.commit()
        return "User deleted successfully", 200

@app.route('/log', methods=['POST'])
def log_entry():
    conn = get_db_connection()
    new_log = request.json
    conn.execute('INSERT INTO log (uid, type, time) VALUES (?, ?, ?)',
                 (new_log['uid'], new_log['type'], datetime.now().strftime('%Y-%m-%d %H:%M:%S')))
    conn.commit()
    return "Log entry added successfully", 201

if __name__ == '__main__':
    app.run(debug=True)