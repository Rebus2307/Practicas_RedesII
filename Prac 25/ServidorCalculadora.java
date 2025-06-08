import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.rmi.Naming;

public class ServidorCalculadora extends UnicastRemoteObject implements Calculadora {

    protected ServidorCalculadora() throws RemoteException {
        super();
    }

    @Override
    public int sumar(int a, int b) throws RemoteException {
        return a + b;
    }

    @Override
    public int restar(int a, int b) throws RemoteException {
        return a - b;
    }

    @Override
    public int multiplicar(int a, int b) throws RemoteException {
        return a * b;
    }

    @Override
    public double dividir(int a, int b) throws RemoteException {
        if (b == 0) {
            throw new RemoteException("No se puede dividir entre cero");
        }
        return (double) a / b;
    }

    public static void main(String[] args) {
        try {
            ServidorCalculadora servidor = new ServidorCalculadora();
            Naming.rebind("ServicioCalculadora", servidor);
            System.out.println("Servidor de Calculadora en funcionamiento...");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
