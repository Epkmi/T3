package com.bcopstein.endpointsdemo1;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/biblioteca/listas")
public class ListaController {

    private final ListaService listaService;

    @Autowired
    public ListaController(ListaService listaService) {
        this.listaService = listaService;
    }

    @GetMapping("/contarObrasDoAutor")
    public long contarObrasDoAutor(@RequestParam String autor) {
        return listaService.contarObrasDoAutor(autor);
    }

    @GetMapping("/contarObrasMaisRecentesQue")
    public long contarObrasMaisRecentesQue(@RequestParam int ano) {
        return listaService.contarObrasMaisRecentesQue(ano);
    }

    @GetMapping("/calcularNumeroMedioObrasPorAutor")
    public double calcularNumeroMedioObrasPorAutor() {
        return listaService.calcularNumeroMedioObrasPorAutor();
    }
}
