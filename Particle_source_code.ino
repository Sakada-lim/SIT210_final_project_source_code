// This #include statement was automatically added by the Particle IDE.
#include <JsonParserGeneratorRK.h>


#include <Adafruit_DHT.h>

//defining the variable for later usage 
#define DHTPIN D6
#define DHTTYPE DHT11
#define Potentiometer A5
int max_range_adjustment = D7;
int min_range_adjustment = D8;

// variable to publish to the cloud

String Data_to_publish; 
String Alert_the_user;

//Backup veriable for the sensor when the sensor read the value incorrectly due to loose wire and the inaccuracy of the sensor DHT11 itself
float backuphumidity = 0;
float backuptemperature = 0;

// this is the counter of how maybe nan it read continuously, and we use this variable to decide whither or not the senosr maybe broken
// this variable will be use in the exception handling function below
int isnan_counter =  0;


//variable to store the potentiometer reading
int potval = 0;

//variables for range of temperature
int max_range = 12;
int min_range = -10;
// function pull from the library to calculation of the temperature and humidity
DHT dht(DHTPIN, DHTTYPE);

void setup() {
    pinMode(max_range_adjustment, INPUT);
    pinMode(min_range_adjustment, INPUT);
    dht.begin();
}

void loop() {

    // read both humidity and temperature from the sensor and convert it using the included library <Adafruit_DHT.h>.
    float humidity = dht.getHumidity();
    float temperature = dht.getTempCelcius();
    int max_range_adjustment_check = digitalRead(max_range_adjustment);
    int min_range_adjustment_check = digitalRead(min_range_adjustment);
    if (max_range_adjustment_check == LOW)
    {
        Serial.println("adjusting max range");
        while(max_range_adjustment_check == LOW)
        {
            max_range_adjustment_check = digitalRead(max_range_adjustment);
            max_range = Adjustment_range();
            Serial.println(max_range);
            delay(1000);
        }
        Serial.print("New max range adjusted to: ");
        Serial.println(max_range);
    }
    else if (min_range_adjustment_check == LOW)
    {
        Serial.println("adjusting min range");
        while(min_range_adjustment_check == LOW)
        {
            min_range_adjustment_check = digitalRead(min_range_adjustment);
            min_range = Adjustment_range();
            Serial.println(min_range); 
            delay(1000);
        }
        Serial.print("New min range adjusted to: ");
        Serial.println(min_range);
    }
    else
    {
        // Call the publish to web function in other for the web to use this data to update the cloud GUI
        publish_to_web(humidity, temperature);
        delay(10000);
    }
}

// This function is responsible for publish data to webhook or other UI and decide what data to upload
// It also have an in-built exception handling to make sure that the temperature is accurate 
void publish_to_web(float humidity, float temperature)
{
    /*
     * Exception handling
     * we use the isnan function to check if the newest humidity is not nan (nan stand for not a number)
     * if it is nan we will return the old value instead of the nan and we increment the isnan counter by 1 each time this happen
     * if the nan counter go over a current amount we will send an alert to the user letting them know that are problems within the system
     * due to the cheap sensor, these problem are likely to occur due to loose wire and other reason by it is around 1 in 10 datas read
     */
    if(isnan(humidity))
    {
        isnan_counter++;
        if(isnan_counter > 5)
        {
            Alert("Broken humidity sensor");
            return;
        }

        Data_to_publish = String::format ("%.1f %, %.1f C", backuphumidity, backuptemperature);
        Serial.println(Data_to_publish);
        createEventPayload(backuptemperature, backuphumidity);
    }
    // this statement will run if the data read is correct. We will set the backuptemperature and humidity to the newest one and also decrement the nan counter
    else
    {
        if(isnan_counter > 0)
        {
            isnan_counter--;
        }
        else
        {
            isnan_counter = 0;
        }
        backuptemperature = temperature;
        backuphumidity = humidity;
        if(temperature > max_range or temperature < min_range)
        {
            Alert("Temperature warning");
        }
        Data_to_publish = String::format ("%.1f %, %.1f C", humidity, temperature);
        Serial.println(Data_to_publish);
        createEventPayload(temperature, humidity);
    }
}
// Alert the user via and email or message to late them know that the temperature have drop or raise above a certain point
void Alert(String message)
{
    Particle.publish("Alert", message, PUBLIC);
}
//mapping function to map the value to the correct reading

//function to adjust the range of temperature
int Adjustment_range()
{
    potval = analogRead(Potentiometer);
    return map(potval, 0, 1023, -10, 0);
}
// this is a Jsonparser function that allow for multiple publish of an event
// we are basically creating an object and than publishing that object on to the web
void createEventPayload(int temp, int humidity)
{
    JsonWriterStatic<256> jw;
    {
        JsonWriterAutoObject obj(&jw);
        jw.insertKeyValue("temp", temp);
        jw.insertKeyValue("humidity", humidity);
    }
    Particle.publish("DHT11", jw.getBuffer(), PUBLIC);
}
