#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DEFAULT_MIN_TEMPERATURE -40
#define DEFAULT_MAX_TEMPERATURE 150
#define TEMPERATURE_STEP 10

void generate_ntc_data(double R0, double T0, double Beta, int orientation, double known_resistance, double VCC, double ADC_ref_voltage, int min_temp, int max_temp, int adc_resolution);

int main(int argc, char *argv[])
{
    double R0, T0, Beta, known_resistance, VCC, ADC_ref_voltage;
    int orientation, min_temp, max_temp, adc_resolution;

    if (argc == 11) {
        R0 = atof(argv[1]);
        T0 = atof(argv[2]);
        Beta = atof(argv[3]);
        orientation = atoi(argv[4]);
        known_resistance = atof(argv[5]);
        VCC = atof(argv[6]);
        ADC_ref_voltage = atof(argv[7]);
        min_temp = atoi(argv[8]);
        max_temp = atoi(argv[9]);
        adc_resolution = atoi(argv[10]);
    } else {
        printf("Enter R0 (room temperature resistance in ohms): ");
        scanf("%lf", &R0);
        printf("Enter T0 (room temperature in Celsius): ");
        scanf("%lf", &T0);
        printf("Enter Beta value: ");
        scanf("%lf", &Beta);
        printf("Enter orientation (0 for pulled down, 1 for pulled up): ");
        scanf("%d", &orientation);
        printf("Enter known resistance value in ohms: ");
        scanf("%lf", &known_resistance);
        printf("Enter supply voltage (VCC) in volts: ");
        scanf("%lf", &VCC);
        printf("Enter ADC reference voltage in volts: ");
        scanf("%lf", &ADC_ref_voltage);
        printf("Enter minimum temperature in Celsius: ");
        scanf("%d", &min_temp);
        printf("Enter maximum temperature in Celsius: ");
        scanf("%d", &max_temp);
        printf("Enter ADC resolution (number of bits): ");
        scanf("%d", &adc_resolution);
    }

    generate_ntc_data(R0, T0, Beta, orientation, known_resistance, VCC, ADC_ref_voltage, min_temp, max_temp, adc_resolution);

    return 0;
}

void generate_ntc_data(double R0, double T0, double Beta, int orientation, double known_resistance, double VCC, double ADC_ref_voltage, int min_temp, int max_temp, int adc_resolution)
{
    FILE *file = fopen("ntc_data.h", "w");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    int adc_max_value = (1 << adc_resolution) - 1;

    fprintf(file, "#ifndef NTC_DATA_H\n");
    fprintf(file, "#define NTC_DATA_H\n\n");
    fprintf(file, "// NTC Data Table\n");
    fprintf(file, "// Temperature (C), Resistance (Ohms), Microvolts, ADC Value\n");

    for (int T = min_temp; T <= max_temp; T += TEMPERATURE_STEP) {
        double T_kelvin = T + 273.15;
        double T0_kelvin = T0 + 273.15;
        double resistance = R0 * exp(Beta * ((1 / T_kelvin) - (1 / T0_kelvin)));

        double voltage;
        if (orientation == 0) { // Pulled down
            voltage = (VCC * resistance) / (resistance + known_resistance);
        } else { // Pulled up
            voltage = VCC - (VCC * resistance) / (resistance + known_resistance);
        }

        double microvolts = (voltage / ADC_ref_voltage) * 1e6;
        int adc_value = (int)((voltage / VCC) * adc_max_value);

        fprintf(file, "%d, %.2f, %.2f, %d\n", T, resistance, microvolts, adc_value);
    }

    fprintf(file, "\n#endif // NTC_DATA_H\n");
    fclose(file);
}
