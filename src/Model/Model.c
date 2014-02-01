#include "Model.h"

ChessBoard * Model_Initialize(void){
	return InitializeChessBoard();
}

/* move piece to next location */
ChessBoard * Model_PerformMove(ChessBoard * board, ChessMoveList * moveList, ChessMove * move)
{
	/* the place to move to has an enemy piece */
	if (move->NextPosition->Piece != NULL)
	{
		/* send to grave yard*/
		move->CapturePiece = move->NextPosition->Piece;
		/* kill it */
		move->NextPosition->Piece->AliveFlag = False;
		/* set that dead piece coordinate to null */
		move->NextPosition->Piece->Coordinate = NULL;
		
		/* it is dead */
		move->CaptureFlag = True;
	}
	/* piece to move, moved to next coordinate */
	move->StartPosition->Piece->Coordinate = move->NextPosition;
	
	/* delete piece pointer at previous location */
	move->StartPosition->Piece = NULL;

	/* update move list */
	ChessMoveNode * newMoveNode = malloc(sizeof(ChessMoveNode));
	assert (newMoveNode);

	/* point new List to old list */
	newMoveNode->List = moveList;

	/* point new move to move */
	newMoveNode->Move = move;
	
	/* set next node to null */
	newMoveNode->NextNode = NULL;

	/* list is empty */
	if (moveList ->FirstNode == NULL)
	{
		moveList->FirstNode = newMoveNode;
		moveList->LastNode = newMoveNode;
		newMoveNode->PrevNode = NULL;
	}

	/* list is not empty */
	else
	{
		moveList->LastNode->NextNode = newMoveNode;
		newMoveNode->PrevNode = moveList->LastNode;
		moveList->LastNode = newMoveNode;
	}
	return board;
}

ChessBoard * Model_UndoLastMove(ChessBoard * board, ChessMoveList * moveList)
{
      int i = 0;
      /* move to delete */
      ChessMoveNode * tempNode = moveList->LastNode;
      
      /* need to move back twice */
      for (i = 0; i < 2; i ++)
      {
	 /* moving back a position (1 undo) */
	tempNode->Move->MovePiece->Coordinate = tempNode->Move->StartPosition;
	tempNode->Move->StartPosition->Piece = tempNode->Move->MovePiece;
	
	/* need to restore a piece from the graveyard */
	if (tempNode->Move->CaptureFlag)
	{
	    /* bring back the dead */
	    tempNode->Move->NextPosition->Piece = tempNode->Move->CapturePiece;
	    tempNode->Move->CapturePiece->Coordinate = tempNode->Move->NextPosition;
	}
	/*nothing to restore from graveyard */
	else
	{
	    tempNode->Move->NextPosition->Piece = NULL;
	}
	/*breaking the forward link of previous node*/
	tempNode->PrevNode->NextNode = NULL;
	/*moving the node back one */
	tempNode = tempNode->PrevNode;
	/*update the last node */
	moveList->LastNode = tempNode;

	free(tempNode->NextNode);
      }
      
      return board;
}
ChessCoordinateList * Model_GetAllLegalCoordinate( ChessBoard * board, ChessPlayer * player, ChessPlayer * PlayerInTurn)
{
	int i = 0;
	
	/* create a permanent coordinate list to be return */
	ChessCoordinateList *newChessCoordinateList1;
	/* create a temp coordinate list to be append */
	ChessCoordinateList *newChessCoordinateList2;

	/* storing the value of the permanent list */
	newChessCoordinateList1 = Model_GetLegalCoordinates(board, player->Pieces[0], PlayerInTurn);

	/* for loop to store the remaining 15 piece into a temp list, then appending new coordinate into 
	the permanent list */
	for (i = 1; i < 16; i++)
	{
		/* storing coordinate into the temp list */
		newChessCoordinateList2 = Model_GetLegalCoordinates(board, player->Pieces[i], PlayerInTurn);
		
		/* appending the two list so there is no duplicate coordinate */
		ChessCoordinateList_AppendNoRedundancy(newChessCoordinateList1, newChessCoordinateList2);
	}
	return newChessCoordinateList1;
}

Boolean Model_CheckCheckedPosition(ChessBoard * board, ChessPlayer * player)
{
	/* grab list of legal move of other player */
	ChessCoordinateList * newList = Model_GetAllLegalCoordinate(board, player->OtherPlayer, player);
	
	/* a temp node to traverse the list */
	ChessCoordinateNode *tempNode = newList->FirstNode;
	ChessPiece * king = ChessPlayer_GetChessPiece(player, King, 0);

	/* there is a legal move for other player */
	if (newList != NULL)
	{
		while (tempNode != NULL)
		{
		/* check the coordinate of the player king to the legal coordinate of other player */
			if (king->Coordinate == tempNode->Coordinate)
			{
				return True;
			}
			/* traverse the list */
			tempNode = tempNode->NextNode;
		}
	}
	return False;
}

Boolean Model_CheckCheckmate(ChessBoard * board, ChessPlayer * player)
{
	
	if (Model_CheckCheckedPosition(board, player) && Model_GetAllLegalCoordinate(board, player, player->OtherPlayer) == NULL)	
	{
		return True;
	}	
	return False;
}

Boolean Model_Stalemate(ChessBoard * board, ChessPlayer * player)
{
	if (!Model_CheckCheckedPosition(board, player) && Model_GetAllLegalCoordinate(board, player, player->OtherPlayer) == NULL)
	{
		return True;
	}	
	return False;
}

/* create and write to log file */
int writeToLogFile(char fname[100], ChessMoveList * moveList)
{
	/* creating the file */
	FILE *File;
	
	/* file type */
	char ftype[] = ".txt";
	
	/* a copy of the file name */
	char fname_tmp[100];  
	
	/* node pointer to traverses the list */
	ChessMoveNode * temp = moveList->FirstNode;
	
	/* move counter */
	int counter = 1;

	/* copy file name to new file */
    strcpy(fname_tmp, fname);
	
	/* concat the file type to the name */
	strcat(fname_tmp, ftype);

	/* open the file */
	File = fopen(fname_tmp, "w");
	
	/* file could not be open */
	if (!File) {
		printf("Cannot open file \"%s\" for writing!\n", fname);
		return 1;
	}
	
	/* going though the list and printing the move */
	while (temp)
	{
		/*printing the move type */
		fprintf(File, "%s", temp->Move->MoveType);
		
		/* print a special move */
		if (temp->Move->MoveType != Normal)
		{
			
		}
		
		/* print a normal move */
		else
		{
			fprintf(File, "  Move #%d: %s move from %s to %s", counter, 
				temp->Move->MovePiece->Type, temp->Move->StartPosition, temp->Move->NextPosition);
		}
		
		/* print the capture piece */
		if (temp->Move->CaptureFlag == True)
		{
			/* a somewhat special message if the queen is capture */
			if (temp->Move->CapturePiece->Type == Queen)
			{
				fprintf(File, "    %s player has captured the graceful %s", temp->Move->MovePiece->Player->PlayerColor, "Queen");
			}
			
			/* capture message for everything else besides king */
			else
			{
				fprintf(File, "    %s has captured a %s", temp->Move->MovePiece->Player->PlayerColor, temp->Move->CapturePiece->Type);
			}
		}
			
		/* advance the node */
		temp = temp->NextNode;
		
		/* increase the counter */
		counter++;
	}
	
	/* un able to open or an error */
	if (ferror(File)) 
	{
		printf("\nFile error while writing to file!\n");
		return 2;
	}
	
	/* close the file */
	fclose(File);
	
	/*print that the file was saved successfully */
	printf("%s was saved successfully. \n", fname_tmp);

	return (0);
}

/* uses GetLegalCoordinates */
/* see if move is legal */
Boolean Model_CheckLegalMove(ChessBoard * board, ChessMove * moveTo)
{
	return True;
}

void Model_CleanUp(ChessBoard * CurrBoard, ChessMoveList * MoveList){
	ChessMoveNode * MoveNode1, * MoveNode2;
	MoveNode1 = MoveList->FirstNode;
	
	while (MoveNode1){
		MoveNode2 = MoveNode1->NextNode;
		free(MoveNode1->Move);
		free(MoveNode1);
		MoveNode1 = MoveNode2;
	}
		
	free(MoveList);
	
	int i,j;
	for (i = 0; i < CHESS_BOARD_MAX_ROW ; i++){
		for (j = 0; j < CHESS_BOARD_MAX_COL; j++){
			/*free the pieces*/
			if (CurrBoard->Board[i][j]->Piece) free(CurrBoard->Board[i][j]->Piece);
			/*free coordinate*/
			free(CurrBoard->Board[i][j]);
		}
	}
	
	/*free the player*/
	free(CurrBoard->WhitePlayer);
	free(CurrBoard->BlackPlayer);
	
	/*free the board*/
	free(CurrBoard);
	
}
