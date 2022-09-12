#include <mpi.h>
#include <stdio.h>
/* Идентификаторы сообщений */
#define tagFloatData 1
#define tagDoubleData 2

int main(int argc, char** argv)
{
	int size, rank, count, i;
	double t1, t2, t3;
	float floatData[10];
	double doubleData[20];
	MPI_Status status;
	/* Инициализациябиблиотеки MPI*/
	MPI_Init(&argc, &argv);
	/* Каждая ветвь узнает количество задач в стартовавшем приложении */
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	/* и свой собственный номер: от 0 до (size-1) */
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	/* Пользователь должен запустить ровно две задачи, иначе ошибка */
	if (size != 2)
	{
		/* Задача с номером 0 сообщает пользователю об ошибке */
		if (rank == 0)
			printf("Error: two processes required instead of %d, abort\n", size);
		/* Все ветви в области связи MPI_COMM_WORLD будут стоять, пока ветвь
		0 не выведет сообщение.
		*/
		MPI_Barrier(MPI_COMM_WORLD);
		/* Без точки синхронизации может оказаться, что одна из ветвей вызовет
		MPI_Abort раньше, чем успеет отработать printf() в ветви 0, MPI_Abort немедленно принудительно завершит все ветви и сообщение выведено не будет. Все задачи аварийно завершают работу */
		MPI_Abort(
			MPI_COMM_WORLD, /* Описатель области связи, на которую распространяется действие ошибки */
			MPI_ERR_OTHER); /* Целочисленный код ошибки */
		return -1;
	}

	if (rank == 0)
	{
		for (i = 0;i < 10;i++)floatData[i] = i;for (i = 0;i < 20;i++)doubleData[i] = -i;
		/* Передача из ветви 0 в ветвь 1 */
		MPI_Send(
			floatData, /* адрес передаваемого массива */
			5, /* передано 5 элементов, т.е.
			floatData[0]..floatData[4] */
			MPI_FLOAT, /* тип переданных элементов */
			1, /* кому: ветви 1 */
			tagFloatData, /* идентификатор сообщения */
			MPI_COMM_WORLD); /* описатель области связи, черезкоторую происходит передача */
			/* и еще одна передача: данные другого типа */
		t1 = MPI_Wtime();

		MPI_Send(doubleData, 6, MPI_DOUBLE, 1, tagDoubleData,
			MPI_COMM_WORLD);
		t1 = MPI_Wtime() - t1;

	}
	else
	{
		/* Ветвь 1 принимает данные от ветви 0,
		дожидается сообщения и помещает пришедшие данные в буфер */
		MPI_Recv(
			floatData, /* адрес массива приемного буфера */
			10, /*максимальная длина приемного буфера */
			MPI_FLOAT, /* сообщаем MPI, что пришедшее
			сообщение состоит из чисел
			типа 'float' */
			0, /* от кого: от ветви 0 */
			tagFloatData, /* ожидаем сообщение с таким идентификатором */
			MPI_COMM_WORLD, /* описатель области связи, через
			которую ожидается приход сообщения */
			&status); /* сюда будет записан статус завершения
			приема */
			/* Вычисляем фактически принятое количество данных */
		MPI_Get_count(
			&status, /* статус завершения приема в MPI_Recv */
			MPI_FLOAT, /* сообщаем MPI, что пришедшее сообщение
			состоит из чисел типа 'float' */
			&count); /* сюда будет записан результат */
		for (i = 0;i < count;i++)if (floatData[i] != i) { printf("Error 1: i=%3d\n", i); }
		/* Выводим фактическую длину принятого на экран */
		printf("rank = %d Received %d elems float\n", rank, count);
		/* Аналогично принимаем сообщение с данными типа double*/
		MPI_Recv(doubleData, 20, MPI_DOUBLE,
			0, tagDoubleData, MPI_COMM_WORLD, &status);
		MPI_Get_count(&status, MPI_DOUBLE, &count);
		for (i = 0;i < count;i++)if (doubleData[i] != -i) { printf("Error 2: i=%3d\n", i); }
		printf("Received %d elems double\n", count);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	/* Обе ветви завершают выполнение */
	MPI_Finalize();
	return 0;
}
