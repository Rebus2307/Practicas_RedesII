import java.rmi.Naming;
import java.util.Scanner;

public class ClienteCalculadora {
    public static void main(String[] args) {
        try {
            Calculadora calculadora = (Calculadora) Naming.lookup("rmi://localhost/ServicioCalculadora");

            Scanner scanner = new Scanner(System.in);
            System.out.print("Ingrese el primer número: ");
            int a = scanner.nextInt();

            System.out.print("Ingrese el segundo número: ");
            int b = scanner.nextInt();

            System.out.println("Suma: " + calculadora.sumar(a, b));
            System.out.println("Resta: " + calculadora.restar(a, b));
            System.out.println("Multiplicación: " + calculadora.multiplicar(a, b));
            System.out.println("División: " + calculadora.dividir(a, b));

            scanner.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
