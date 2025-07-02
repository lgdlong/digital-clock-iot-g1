// config/config-db.ts
import { MongoClient, Db } from "mongodb";

const uri = process.env.MONGODB_URI!;
const dbName = "digital-clock-iot-g1";

let client: MongoClient;
let db: Db;

export async function connectDB() {
  try {
    if (!client || !db) {
      client = new MongoClient(uri);
      await client.connect();
      db = client.db(dbName);
    }
    return db;
  } catch (error) {
    console.error("Failed to connect to MongoDB:", error);
    throw new Error("Database connection failed");
  }
}
