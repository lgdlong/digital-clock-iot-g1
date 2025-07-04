import swaggerJSDoc from 'swagger-jsdoc';

const swaggerDefinition = {
  openapi: '3.0.0',
  info: {
    title: 'ESP32 Smart Clock API',
    version: '1.0.0',
    description: 'API documentation for ESP32 Smart Clock with alarm management and MQTT integration',
    contact: {
      name: 'Digital Clock IoT Team',
      email: 'team@digitalclock.iot'
    },
  },
  servers: [
    {
      url: 'https://digital-clock-iot-g1.vercel.app',
      description: 'Development server',
    },
    {
      url: 'http://localhost:3000',
      description: 'Production server',
    },
  ],
  components: {
    schemas: {
      Alarm: {
        type: 'object',
        required: ['hour', 'minute', 'daysOfWeek', 'enabled'],
        properties: {
          _id: {
            type: 'string',
            description: 'Unique identifier for the alarm',
            example: '60f1b2b3b3b3b3b3b3b3b3b3'
          },
          hour: {
            type: 'integer',
            minimum: 0,
            maximum: 23,
            description: 'Hour of the alarm (24-hour format)',
            example: 7
          },
          minute: {
            type: 'integer',
            minimum: 0,
            maximum: 59,
            description: 'Minute of the alarm',
            example: 30
          },
          daysOfWeek: {
            type: 'array',
            items: {
              type: 'integer',
              minimum: 0,
              maximum: 6
            },
            description: 'Days of the week (0=Sunday, 1=Monday, ..., 6=Saturday)',
            example: [1, 2, 3, 4, 5]
          },
          enabled: {
            type: 'boolean',
            description: 'Whether the alarm is enabled',
            example: true
          },
          label: {
            type: 'string',
            description: 'Optional label for the alarm',
            example: 'Wake up'
          }
        }
      },
      AlarmCreate: {
        type: 'object',
        required: ['hour', 'minute', 'daysOfWeek', 'enabled'],
        properties: {
          hour: {
            type: 'integer',
            minimum: 0,
            maximum: 23,
            description: 'Hour of the alarm (24-hour format)',
            example: 7
          },
          minute: {
            type: 'integer',
            minimum: 0,
            maximum: 59,
            description: 'Minute of the alarm',
            example: 30
          },
          daysOfWeek: {
            type: 'array',
            items: {
              type: 'integer',
              minimum: 0,
              maximum: 6
            },
            description: 'Days of the week (0=Sunday, 1=Monday, ..., 6=Saturday)',
            example: [1, 2, 3, 4, 5]
          },
          enabled: {
            type: 'boolean',
            description: 'Whether the alarm is enabled',
            example: true
          },
          label: {
            type: 'string',
            description: 'Optional label for the alarm',
            example: 'Wake up'
          }
        }
      },
      Error: {
        type: 'object',
        properties: {
          error: {
            type: 'string',
            description: 'Error message',
            example: 'Invalid alarm data'
          }
        }
      },
      Success: {
        type: 'object',
        properties: {
          success: {
            type: 'boolean',
            description: 'Success status',
            example: true
          }
        }
      },
      ESP32Time: {
        type: 'object',
        properties: {
          time: {
            type: 'string',
            description: 'Current time from ESP32 in HH:MM:SS format',
            example: '14:30:25'
          },
          timestamp: {
            type: 'number',
            description: 'Unix timestamp when data was received',
            example: 1635768625000
          },
          source: {
            type: 'string',
            description: 'Data source',
            example: 'ESP32'
          },
          status: {
            type: 'string',
            description: 'Connection status',
            example: 'connected'
          }
        }
      },
      ESP32Status: {
        type: 'object',
        properties: {
          connected: {
            type: 'boolean',
            description: 'Whether ESP32 is connected',
            example: true
          },
          mqttBroker: {
            type: 'string',
            description: 'MQTT broker URL',
            example: 'broker.hivemq.com:1883'
          },
          latency: {
            type: 'number',
            description: 'Connection latency in milliseconds',
            example: 150
          },
          timestamp: {
            type: 'number',
            description: 'Test timestamp',
            example: 1635768625000
          },
          topics: {
            type: 'object',
            properties: {
              timeTopic: {
                type: 'string',
                example: 'clock/time'
              },
              resetTopic: {
                type: 'string',
                example: 'clock/reset'
              }
            }
          },
          message: {
            type: 'string',
            description: 'Status message',
            example: 'MQTT broker connection test successful'
          },
          error: {
            type: 'string',
            description: 'Error message if connection failed',
            example: 'Connection timeout'
          }
        }
      }
    }
  }
};

const options = {
  definition: swaggerDefinition,
  apis: [
    './src/app/api/alarms/route.ts',
    './src/app/api/set-alarm/route.ts', 
    './src/app/api/delete-alarm/route.ts',
    './src/app/api/update-alarm/route.ts',
    './src/app/api/esp32-time/route.ts',
    './src/app/api/esp32-status/route.ts'
  ], // Path to the API files
};

const specs = swaggerJSDoc(options);

export default specs;
