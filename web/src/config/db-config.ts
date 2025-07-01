// config/config-db.ts
import { MongoClient, Db } from "mongodb";

const uri = process.env.MONGODB_URI!;
const dbName = "digital-clock-iot-g1";

let client: MongoClient;
let db: Db;

export async function connectDB() {
  console.log("MONGODB_URI in db-config.ts:", process.env.MONGODB_URI);
  if (!client || !db) {
    client = new MongoClient(uri);
    await client.connect();
    db = client.db(dbName);

    console.log("MONGODB_URI:", process.env.MONGODB_URI);
  }
  return db;
}
