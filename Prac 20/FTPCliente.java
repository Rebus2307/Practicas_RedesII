import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.*;
import java.net.Socket;

public class FTPCliente extends JFrame {
    private static final String SERVER_ADDRESS = "192.168.1.100"; // Cambia por la IP de tu servidor Linux Mint
    private static final int PORT = 21;
    private JTextField filePathField;
    private JButton uploadButton;

    public FTPCliente() {
        setTitle("Cliente FTP");
        setSize(400, 200);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLayout(new FlowLayout());

        filePathField = new JTextField(20);
        uploadButton = new JButton("Seleccionar y Subir Archivo");

        uploadButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                selectFile();
            }
        });

        add(filePathField);
        add(uploadButton);
    }

    private void selectFile() {
        JFileChooser fileChooser = new JFileChooser();
        int returnValue = fileChooser.showOpenDialog(this);

        if (returnValue == JFileChooser.APPROVE_OPTION) {
            File selectedFile = fileChooser.getSelectedFile();
            filePathField.setText(selectedFile.getAbsolutePath());
            uploadFile(selectedFile.getAbsolutePath());
        }
    }

    public void uploadFile(String filePath) {
        try (Socket socket = new Socket(SERVER_ADDRESS, PORT);
             OutputStream output = socket.getOutputStream();
             InputStream input = socket.getInputStream();
             PrintWriter writer = new PrintWriter(output, true);
             BufferedReader reader = new BufferedReader(new InputStreamReader(input))) {

            File file = new File(filePath);
            writer.println("PUT " + file.getName());

            String serverResponse = reader.readLine();
            if ("READY".equals(serverResponse)) {
                System.out.println("Servidor listo para recibir el archivo.");
                sendFile(file, output);
                writer.println("END");
                String finalResponse = reader.readLine();
                System.out.println("Respuesta final del servidor: " + finalResponse);
            } else {
                System.out.println("Error: El servidor no está listo. Respuesta: " + serverResponse);
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void sendFile(File file, OutputStream output) throws IOException {
        try (FileInputStream fileInput = new FileInputStream(file)) {
            byte[] buffer = new byte[4096];
            int bytesRead;

            System.out.println("Enviando archivo: " + file.getName());

            while ((bytesRead = fileInput.read(buffer)) != -1) {
                output.write(buffer, 0, bytesRead);
            }

            output.flush();
            System.out.println("Archivo " + file.getName() + " enviado con éxito.");
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            FTPCliente cliente = new FTPCliente();
            cliente.setVisible(true);
        });
    }
}
