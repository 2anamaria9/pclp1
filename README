//image_editor

	Programul propus pune in vedere gestionarea atat a fisierelor text, cat si a celor binare, in vederea manipularii imaginilor digitale.
	Am ales sa ma folosesc de o structura definita image, care poate retine valori pentru componentele de culoare (R, G, B) sau intensitatea pixelilor pentru imagini alb-negru, imaginea fiind reprezentata ca o matrice de astfel de structuri.
	Utilizatorul dispune de o serie de comenzi cu ajutorul carora poate interactiona cu imaginile:
	*comanda LOAD, ce are drept rol incarcarea imaginii; se realizeaza citirea fisierului, determinandu-se astfel tipul si dimensiunea imaginii, valoarea maxima a pixelilor, iar apoi pixelii in sine, stocati intr-o matrice alocata dinamic;
	*comanda SELECT permite selectarea unei regiuni a imaginii, inaintea careia are loc validarea coordonatelor;
	*comanda SELECT ALL are rolul de a selecta intreaga imagine;
	*comanda CROP are rolul de a elimina zonele neincluse in selectie si de a reduce dimensiunea imaginii la cea a selectiei;
	*comanda ROTATE realizeaza rotirea ori a unei selectii patrate, ori a intregii imagini in multiplii de 90 de grade utilizand o matrice auxiliara si actualizand permanent imaginea originala si dimensiunile sale;
	*comanda APPLY, ce functioneaza doar asupra imaginilor color, atribuie imaginii diverse filtre precum EDGE, SHARPEN, GAUSSIAN_BLUR, BLUR prin construirea kernel-ului adecvat si aplicarea acestuia asupra imaginii;
	*comanda HISTOGRAM, ce functioneaza doar asupra imaginilor alb-negru, calculeaza histograma imaginii utilizand un numar definit de binuri, rezultand un grafic alcatuit din stelute;
	*comanda EQUALIZE, ce functioneaza doar asupra imaginilor alb-negru, ajusteaza distributia intensitatilor pixelilor pentru a obtine o imagine cu contrast îmbunatatit, folosindu-se de histograma;
	*comanda SAVE permite scrierea imaginii intr-un fisier specificat, al carui nume este extras din intreaga comanda si salvarea in mod ascii sau binar, in functie de optiunea specificata
	*comanda EXIT opreste introducerea oricarei comenzi 
	Fiecare comanda este implementata ca o functie separata, la care am adaugat cateva functii precum free_pixels (pentru golirea memoriei), clamp (pentru impunerea valorilor de a se incadra in intervalul [0,255]) si build_kernel (pentru constructia kernel-ului potrivit), ultimele doua fiind necesare functiei apply.
	Totodata, am avut in vedere efectuarea alocarilor si dealocarilor de memorie in mod corespunzator si verificarea oricaror erori generate de alocarea memoriei sau de alte conditii invalide.
	
	
