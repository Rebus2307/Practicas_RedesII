import tkinter as tk
from tkinter import messagebox, filedialog
import xmlrpc.client
import base64

proxy = xmlrpc.client.ServerProxy("http://<IP_SERVIDOR>:8000")

def listar_archivos():
    try:
        archivos = proxy.list_files(".")
        listbox.delete(0, tk.END)
        for archivo in archivos:
            listbox.insert(tk.END, archivo)
    except Exception as e:
        messagebox.showerror("Error", f"No se pudieron listar los archivos: {e}")

def leer_archivo():
    archivo_seleccionado = listbox.get(tk.ACTIVE)
    if archivo_seleccionado:
        try:
            contenido = proxy.read_file(archivo_seleccionado)
            text_area.delete(1.0, tk.END)
            text_area.insert(tk.END, contenido)
        except Exception as e:
            messagebox.showerror("Error", f"No se pudo leer el archivo: {e}")
    else:
        messagebox.showwarning("Advertencia", "Selecciona un archivo para leer.")

def enviar_archivo():
    archivo_seleccionado = filedialog.askopenfilename()
    if archivo_seleccionado:
        try:
            with open(archivo_seleccionado, "rb") as file:
                archivo_codificado = base64.b64encode(file.read()).decode('utf-8')
            nombre_archivo = archivo_seleccionado.split("/")[-1]
            resultado = proxy.upload_file(nombre_archivo, archivo_codificado)
            messagebox.showinfo("Éxito", resultado)
            listar_archivos()
        except Exception as e:
            messagebox.showerror("Error", f"No se pudo enviar el archivo: {e}")

root = tk.Tk()
root.title("Cliente RPC")

listbox = tk.Listbox(root, width=50)
listbox.pack()

btn_listar = tk.Button(root, text="Listar Archivos", command=listar_archivos)
btn_listar.pack()

btn_leer = tk.Button(root, text="Leer Archivo", command=leer_archivo)
btn_leer.pack()

btn_subir = tk.Button(root, text="Subir Archivo", command=enviar_archivo)
btn_subir.pack()

text_area = tk.Text(root, height=15, width=60)
text_area.pack()

root.mainloop()
