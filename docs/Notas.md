## Módulo Relé
### Portas de Entrada
- **GND:** Terra
- **IN1:** Controla o relé 1 **ACTIVE LOW**
- **IN2:** Controla o relé 2 **ACTIVE LOW**
- **VCC:** Manda 5V para a porta, usado para controlar os *optoacopladores*
### Portas de Saída
- **COM:** Usada para conectar as portas
- **NO (Normally Open):** Ligada ao **COM** quando o relé está desligado (IN = HIGH)
- **NC (Normally Closed):** Ligada ao **COM** quando o relé está ligado (IN = LOW)

### Analogia
Uma analogia com um circuito digital pode ser
```Verilog
module RELAY(IN, NO, COM, NC);
	input wire IN;
	output wire NO, COM, NC;
	
	assign COM = 1; // está sempre ligado desde que o circuito esteja alimentado
	assign NO = !IN; // active low
	assign NC = IN;
endmodule
// Tecnicamente, as saídas teriam que ficar com alta impedância 'Z' para quando
// não estivessem ligadas, mas é só uma analogia
```
### Observações
- **Optoacoplador:** é um componente que serve para transmitir sinais elétricos sem usar a mesma corrente usando luz.