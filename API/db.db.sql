BEGIN TRANSACTION;
CREATE TABLE IF NOT EXISTS "felhasznalok" (
	"id"	INTEGER NOT NULL UNIQUE,
	"nev"	TEXT NOT NULL,
	"kreditsz"	INTEGER,
	"jog"	INTEGER NOT NULL CHECK("jog" IN (0, 1)),
	PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE TABLE IF NOT EXISTS "log" (
	"id"	INTEGER NOT NULL UNIQUE,
	"uid"	INTEGER NOT NULL,
	"type"	INTEGER NOT NULL,
	"time"	TEXT NOT NULL,
	PRIMARY KEY("id" AUTOINCREMENT),
	FOREIGN KEY("uid") REFERENCES "felhasznalok"("id") ON DELETE SET NULL
);
COMMIT;